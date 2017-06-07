//
//  TapManager.cpp
//  Metro
//
//  Created by Ryan Stables on 24/02/2017.
//
//
#include "TapManager.hpp"

//==============================================================================
//========= Tapper =============================================================
//==============================================================================
Tapper::Tapper()
{
    // some default values to be overwritten
    TKNoiseMS = 25;     //25 ms timekeeper noise
    MNoise    = 10;     //10 ms Motor noise
    setNoteLen(256);
    setVel(127);
    setFreq(60);        // in MidiNotes
}

Tapper::~Tapper()
{
        // nothing to free up here
}

void Tapper::turnNoteOn(MidiBuffer &midiMessages, int sampleNo, Counter globalCounter, bool updateMidiInOutputBuffer)
{
    if(!noteActive)
    {
        // report the noteOn time in samples...
        printTapTime(globalCounter, "NoteOn");
        if(updateMidiInOutputBuffer) // this can be set to false so the input tapper doesn't write out midi messages
            midiMessages.addEvent(MidiMessage::noteOn (MIDIChannel, tapperFreq, (uint8)tapperVel), sampleNo);
        noteActive=true;
        noteNumber++;
    }
}

void Tapper::turnNoteOff(MidiBuffer &midiMessages, int sampleNo, Counter globalCounter, bool updateMidiInOutputBuffer)
{
    // report thee noteOff time in samples...
    if(noteActive)
    {
//        printTapTime(globalCounter, "NoteOff");
        midiMessages.addEvent(MidiMessage::noteOff (MIDIChannel, tapperFreq, (uint8)tapperVel), sampleNo);
        noteActive=false;
        resetOffsetCounter();
    }
}

// set all notes off in the current midi channel...
void Tapper::kill(MidiBuffer &midiMessages)
{
    int lastEventTime = midiMessages.getLastEventTime();
    // find the last tap time and add a note-off after it
    midiMessages.addEvent(MidiMessage::allNotesOff(MIDIChannel), lastEventTime+1);
    noteActive=false;
}

bool Tapper::requiresNoteOn(Counter inputCounter)
{
    if(inputCounter.inSamples()%interval==0 && inputCounter.inSamples() && !noteActive)
        return true;
    else
        return false;
}

bool Tapper::requiresNoteOff()
{
    if (offseCounter.inSamples()>=noteLen && noteActive)
        return true;
    else
        return false;
}

void Tapper::iterate(MidiBuffer & midiMessages, int sampleNum, Counter &globalCounter, std::vector <bool> &notesTriggered)
{
    // check to see if noteOns/Offs need to be added....
    if(requiresNoteOn(globalCounter))
    {
        turnNoteOn(midiMessages, sampleNum, globalCounter, true);
        onsetTime.set(globalCounter.inSamples());
        notesTriggered[tapperID-1] = true;  // this gets turned off by the tapManager when all notes have been triggered
                                            // thiis is i-1 as these are only synths and 0 is the input.
    }
    else if(requiresNoteOff())
    {
        turnNoteOff(midiMessages, sampleNum, globalCounter, true);
    }
    
    if(noteActive) // if note is turned on, start counting towards the noteOff...
        offseCounter.iterate();
}


void Tapper::updateParameters(int ID, int channel, int freq, int noteLen, int interval, int velocity)
{
    // function to update all parameters at once. Used for tapGen initialisation and in processBlock
    setID(ID);
    setChannel(channel); // the first one is the input tapper, start from chan 2
    setFreq(freq); //octaves above middle C
    setNoteLen(noteLen);
    setInterval(interval);
    setVel(velocity);
}

void Tapper::printTapTime(Counter globalCounter, String eventType)
{
    // use this to write the event to the log...
    Logger::outputDebugString("ID: "+String(tapperID)+", Channel: "+String(MIDIChannel)+",  "+eventType+": "+String(globalCounter.inSamples()));
}


//==============================================================================
//========= Generator ==========================================================
//==============================================================================
TapGenerator::TapGenerator(int NumTappers, double sampleRate, int samplesPerBlock)
{
    // store number of tappers...
    numSynthesizedTappers = NumTappers-1;
    fs = sampleRate;
    frameLen = samplesPerBlock;
    
    // allocate the tappers...
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        synthesizedTappers.add(new Tapper);
    }

    // init parameters of the input tapper...
    inputTapper.updateParameters(0 /*ID*/, 1 /*channel*/, 48 /*freq*/, 22050 /*noteLen*/, 44100 /*interval*/, 127 /*velocity*/);

    // init the synthesized tappers.
    for(int i=0; i<numSynthesizedTappers; i++)
    {   // init parameters for all synthesized tappers...
        notesTriggered.push_back(false);
        
        prevAsynch.push_back(0);
        prevTapTimes.push_back(0);
        
        synthesizedTappers[i]->updateParameters(i+1 /*ID*/, i+2 /*channel*/, 60+(i*12) /*freq*/, 22050 /*noteLen*/, 44100 /*interval*/, 127 /*velocity*/);
    }
}


TapGenerator::~TapGenerator()
{
}

void TapGenerator::updateInputTapper(MidiBuffer &midiMessages, Counter globalCounter)
{
    // if the block has MIDI events in it...
    if(!midiMessages.isEmpty())
    {
        // iterate through the events...
        MidiBuffer::Iterator messages(midiMessages);
        MidiMessage result;
        int samplePos;
        
        // get all of the midi messages in the buffer...
        while(messages.getNextEvent(result, samplePos))
        {
            if(result.isNoteOn() && inputTapper.getChannel() == result.getChannel())
            {
                inputTapper.turnNoteOn(midiMessages, samplePos, globalCounter, false);
                userInputDetected = true; // tell the tapManager that a noteOn has been registered.
                numberOfInputTaps.iterate(); // count the number of taps that have been logged
            }
            else if(result.isNoteOff() && inputTapper.getChannel() == result.getChannel())
            {
                inputTapper.turnNoteOff(midiMessages, samplePos, globalCounter, false);
            }
        }
    }
}

void TapGenerator::updateTapAcceptanceWindow()
{
    //subtract mean of current beat times from mean of prev.
    double currentMean=0, prevMean=0;
    
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        currentMean+=(synthesizedTappers[i]->getOnsetTime()/(double)numSynthesizedTappers);
        prevMean+=(prevTapTimes[i]/(double)numSynthesizedTappers);
        
        //assign current to prev tap times.
        prevTapTimes[i] = synthesizedTappers[i]->getOnsetTime();
    }
    
    inputTapAcceptanceWindow = currentMean-prevMean;
    nextWindowThreshold = currentMean + inputTapAcceptanceWindow / 2;
//    Logger::outputDebugString("curr: ["+String(currentMean)+"] + prev: ["+String(prevMean)+"] = mean: ["+String(inputTapAcceptanceWindow)+"]");
    Logger::outputDebugString("next Thresh: "+String(nextWindowThreshold)+"\n");
}



void TapGenerator::resetTriggeredFlags()
{
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        // reset the noteTriggered flags...
        notesTriggered[i] = false;
    }
    // and reset the input detected flag...
    userInputDetected = false;
}

bool TapGenerator::allNotesHaveBeenTriggered()
{
    return std::all_of(notesTriggered.cbegin(), notesTriggered.cend(), [](bool i){ return i==true; });
}


void TapGenerator::updateBPM(double x)
{
    bpm = x;
    TKInterval = static_cast <int> (floor(60.f/bpm*fs));
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        synthesizedTappers[i]->setInterval(TKInterval);
        synthesizedTappers[i]->setNoteLen(TKInterval/beatDivision);
    }
}


void TapGenerator::transform()
{
     int randWindowMs = 5;
     int randWinInSamples = (randWindowMs/1000.0)*fs;                                    // amount of randomness to be added
    
    // return the interval with perturbation...
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        // generate scaled and offset random number...
        int randomValue = rand.nextInt(randWinInSamples) - (randWinInSamples/2);
        // apply it to the interval and compensate for the previous asynch...
        synthesizedTappers[i]->setInterval(synthesizedTappers[i]->getInterval()+randomValue - prevAsynch[i]);
        // update the prevAsynch values...
        prevAsynch[i] = randomValue;
    }
}

bool TapGenerator::inputWindowExists()
{
    if(beatCounter.inSamples() && inputTapAcceptanceWindow)
        return true;
    else
        return false;
}




// run in each process bock to update the note on/offs...
void TapGenerator::nextBlock(MidiBuffer &midiMessages, Counter &globalCounter)
{
    //update the input tapper with incoming on/off messages...
    updateInputTapper(midiMessages, globalCounter);
    
    // GLOBAL SAMPLE COUNTER LOOP --------------------------------------
    for (int sampleNum=0; sampleNum<frameLen; sampleNum++)
    {
        // iterate the synthesized tappers...
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)
            synthesizedTappers[tapperNum]->iterate(midiMessages, sampleNum, globalCounter, notesTriggered);
        
        // BEAT COUNTER -------------------------------------------
        if(allNotesHaveBeenTriggered())
        {
            // use beat counter to start from the second event?...
            
            if(userInputDetected) // this means all notes (incl a user input) have been logged here.
            {
                // DEBUG -----------
                Logger::outputDebugString("Beat ["+String(beatCounter.inSamples())+"] user input found");
                // -----------------
                
                // updateTapAcceptanceWindow(); // be careful - put this is the right place!
                // calculate timing params for next event
                // apply transform()
                // reset the waitforthesh flag and resetTriggeredFlags();
                beatCounter.iterate(); // count the beats
                updateTapAcceptanceWindow();
                resetTriggeredFlags();
            }
            else
            {
                if(globalCounter.inSamples() >= nextWindowThreshold)
                {
                    // DEBUG -----------
                    Logger::outputDebugString("Beat ["+String(beatCounter.inSamples())+"] threshold reached");
                    // -----------------
                    
                    // window thresh has been reached...
                    updateTapAcceptanceWindow(); //<--- calculate the next window thresh here
                    resetTriggeredFlags();
                    beatCounter.iterate(); // count the beats
                }
                else
                {
                    //keep counting towards the thresh...

                }
            }
        }
        
            // Q: Deal with the first beat (no window params)?
                        // set the initial window threshold to something realistic, iherit from bpm or just ignore beat 1?
            // Q: how do i deal wit double taps? - only register a tap if the inputflag = false
            // look into FIFO in JUCE.
        
        
        globalCounter.iterate();
    }
}




// Kill the taps that get left on if the playhead stops whilst a note is active...
void TapGenerator::killActiveTappers(MidiBuffer &midiMessages)
{
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        if(synthesizedTappers[i]->isActive())
        {
            synthesizedTappers[i]->kill(midiMessages);
        }
    }
}

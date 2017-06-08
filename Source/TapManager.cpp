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
        onsetTime.set(globalCounter.inSamples());
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
    
    // open the logfile/stream (this will be moved when i create a 'filename' text input)
    Time time;
    File logFile ("/Users/ryanstables/Desktop/log.txt");
    logFile.appendText("%% ----------------------------\n%% Trial: "+time.getCurrentTime().toString(true, true));
    logFile.appendText("fs = "+String(fs)+";\n");
    logFile.appendText("numTappers = "+String(NumTappers)+";\n");
    logFile.appendText("startingPitches = [NaN");
    
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
        int pitch = 60+(i*12);
        synthesizedTappers[i]->updateParameters(i+1 /*ID*/, i+2 /*channel*/, pitch /*freq*/, 22050 /*noteLen*/, 44100 /*interval*/, 127 /*velocity*/);
        logFile.appendText(" "+String(pitch));
    }
    
    logFile.appendText("];\n");
    logFile.appendText("x = [\n");
    // to stream text to the log file whilst tapping...
    captainsLog = new FileOutputStream (logFile);
}

TapGenerator::~TapGenerator()
{
    captainsLog->writeText("];\n%% ----------------------------\n\n", false, false);
    captainsLog->flush();
}


void TapGenerator::reset()
{
    // reset all of the counters...
    // reset theTKinterval...
    // reset the tappers...
}


void TapGenerator::updateInputTapper(MidiBuffer &midiMessages, Counter globalCounter)
{
    // if the block has MIDI events in it and a note hasn't already been registered in this window...
    if(!midiMessages.isEmpty() && !userInputDetected)
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
    if(beatCounter.inSamples())
    {
        double currentMean=0, prevMean=0;
        
        for (int i=0; i<numSynthesizedTappers; i++)
        {
            // find the mean of the current and prev tap times...
            currentMean+=(synthesizedTappers[i]->getOnsetTime()/(double)numSynthesizedTappers);
            prevMean+=(prevTapTimes[i]/(double)numSynthesizedTappers);
            
            //assign current to prev tap times.
            prevTapTimes[i] = synthesizedTappers[i]->getOnsetTime();
        }
        
        inputTapAcceptanceWindow = currentMean-prevMean;
        nextWindowThreshold = currentMean + inputTapAcceptanceWindow / 2;
    //    Logger::outputDebugString("curr: ["+String(currentMean)+"] + prev: ["+String(prevMean)+"] = mean: ["+String(inputTapAcceptanceWindow)+"]");
        Logger::outputDebugString("Next Thresh: "+String(nextWindowThreshold)+"\n");
    }
    else
    {
        // this should only happen on the first beat.
        nextWindowThreshold = TKInterval*1.5;
        Logger::outputDebugString("Next Thresh: "+String(nextWindowThreshold)+"\n");
    }
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

void TapGenerator::logResults(String inputString)
{
    // write to terminal
    Logger::outputDebugString(inputString);
    
    String inputOnsetTime = "NaN ";
    
    if (userInputDetected)
        inputOnsetTime = String(inputTapper.getOnsetTime());
    
    captainsLog->writeText(inputOnsetTime, false, false);
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        captainsLog->writeText(" "+String(synthesizedTappers[i]->getOnsetTime()), false, false);
    }
    
    captainsLog->writeText(";\n", false, false);
    
//    if(beatCounter.inSamples() % 3 == 0)
//        captainsLog->flush();
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
            if(userInputDetected)
            {
                // Recalculate timing params with all registered asynch values...
                transform();
                logResults("Beat ["+String(beatCounter.inSamples())+"] user input ["+String(numberOfInputTaps.inSamples())+"] found");
                beatCounter.iterate(); // count the beats
                updateTapAcceptanceWindow();
                resetTriggeredFlags();
            }
            else
            {
                if(globalCounter.inSamples() >= nextWindowThreshold)
                {
                    // Recalculate timing params without the user input asnynch...
                    logResults("Beat ["+String(beatCounter.inSamples())+"] threshold reached");
                    updateTapAcceptanceWindow(); //<--- calculate the next window thresh here
                    resetTriggeredFlags();
                    beatCounter.iterate(); // count the beats
                }
                else
                {
                    //Counting towards the next beat threshold...
                }
            }
        }
        
            // Q: how do i deal wit double taps? - only register a tap if the inputflag = false
            // should all counters reset when the DAW stos playing? - beatCounter needs to at least!
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

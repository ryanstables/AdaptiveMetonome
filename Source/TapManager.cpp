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
    // set some default vals using reset...
    reset();
}

Tapper::~Tapper()
{
    // nothing to free up here yet...
}


void Tapper::reset()
{
    // reset to default vals...
    TKNoiseStd      = 0;     // 25 ms timekeeper noise
    MNoiseStd       = 0;     // 10 ms Motor noise
    MNoisePrevValue = 0;      // no prev motor noise
    setNoteLen(256);          // in samples
    setVel(127);              // in MidiNotes
    setFreq(60);              // ...
    
    // reset all the counters...
    countdownToOffset.reset();
    onsetTime.reset();
    numberOfNoteOns.reset();
    numberOfNoteOffs.reset();
}

void Tapper::turnNoteOn(MidiBuffer &midiMessages, int sampleNo, Counter globalCounter, bool updateMidiInOutputBuffer)
{
    if(!noteActive)
    {
        onsetTime.set(globalCounter.inSamples());
        // report the noteOn time in samples...
        printTapTime(globalCounter, "NoteOn");
        
        if(updateMidiInOutputBuffer) // this can be set to false so the input tapper doesn't write out midi messages
        {
            midiMessages.addEvent(MidiMessage::noteOn (MIDIChannel, tapperFreq, (uint8)tapperVel), sampleNo);
        }
        noteActive=true;
        numberOfNoteOns.iterate(); //update the number of NoteOffs to have happened to this tapper
    }
}

void Tapper::turnNoteOff(MidiBuffer &midiMessages, int sampleNo, Counter globalCounter, bool updateMidiInOutputBuffer)
{
    // report thee noteOff time in samples...
    if(noteActive)
    {
        if(updateMidiInOutputBuffer) // this can be set to false so the input tapper doesn't write out midi messages
            midiMessages.addEvent(MidiMessage::noteOff (MIDIChannel, tapperFreq, (uint8)tapperVel), sampleNo);
        
        noteActive=false;
        resetOffsetCounter();
        numberOfNoteOffs.iterate(); //update the number of NoteOffs to have happened to this tapper
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
    if (countdownToOffset.inSamples()>=noteLen && noteActive)
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
        countdownToOffset.iterate();
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
TapGenerator::TapGenerator(int NumTappers, double sampleRate, int samplesPerBlock, String dataPath)
{
    
    setLocalDataPath(dataPath);
    
    // store number of tappers...
    numSynthesizedTappers = NumTappers-1;
    fs = sampleRate;
    frameLen = samplesPerBlock;
    
    // open the logfile/stream (this will be moved when i create a 'filename' text input)
    Time time;
    
    File logFile (localDataPath+"savedData.m");
    logFile.appendText("%% ----------------------------\n%% "+time.getCurrentTime().toString(true, true)+"\n");
    logFile.appendText("fs = "+String(fs)+";\n");
    logFile.appendText("numTappers = "+String(NumTappers)+";\n");
        
    // allocate the tappers...
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        synthesizedTappers.add(new Tapper);
    }

    // to be loaded in from external file or UI
    double tempAlphas[4][4] =
    {
        { 0,   .25,  .25,  .25},
        {.25,  .0,   .25,  .25},
        {.0,   .0,   .0,   .0},
        {.25,  .25,  .25,  .0}
    };
    
    // init alpha and [t(n-1,i)-t(n-1,j)]..
    for (int i=0; i<NumTappers; i++)
    {
        alpha.add(new Array<double>);
        asynch.add(new Array<double>);
        asynchAlpha.add(new Array<double>);

        for (int j=0; j<NumTappers; j++)
        {
            alpha[i]->add(tempAlphas[i][j]); //load from file!
            asynch[i]->add(0.f);
            asynchAlpha[i]->add(0.f);
        }
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
    }
    

    logFile.appendText("\n% Trial: "+String(trialNum.inSamples())+"\n");
    logFile.appendText("x_0 = [\n");
    // to stream text to the log file whilst tapping...
    captainsLog = new FileOutputStream (logFile);
}

TapGenerator::~TapGenerator()
{
    // Write to the the end of the file...
    captainsLog->writeText("];\n%% ----------------------------\n\n", false, false);
    captainsLog->flush();
}


void TapGenerator::reset()
{
    // reset all of the counters...
    beatCounter.reset();
    numberOfInputTaps.reset();
    

    // reset the tappers...
    // ... make sure to reset the numberOfNoteOffs/ons as this is what iterates the midi file!
    for (int tapper=0; tapper<numSynthesizedTappers; tapper++)
    {
        synthesizedTappers[tapper]->reset();
    }

    // reset theTKinterval etc...
    updateBPM(bpm);
    
    // Write to the the end of the file...
    trialNum.iterate();
    captainsLog->writeText("];\n\n% Trial: "+String(trialNum.inSamples())+"\nx_"+String(trialNum.inSamples())+"=[\n", false, false);
    captainsLog->flush();
}

void TapGenerator::readPitchListFromMidiSeq(const OwnedArray<MidiMessageSequence> &inputMIDISeq)
{
    int numTracks = inputMIDISeq.size();
    
    for (int trackNum=0; trackNum < numTracks; trackNum++)
    {
        pitchList.add(new Array<double>);
        int numEvents = inputMIDISeq[trackNum]->getNumEvents();
        for (int eventNum=0; eventNum<numEvents; eventNum++)
        {
            MidiMessageSequence::MidiEventHolder *tempEventHolder = inputMIDISeq[trackNum]->getEventPointer(eventNum);
            
            if(tempEventHolder->message.isNoteOn())
            {
                double pitch = tempEventHolder->message.getNoteNumber();
                pitchList[trackNum]->add(pitch);
            }
        }
    }
}


void TapGenerator::printPitchList()
{
    for (int i=0; i<pitchList.size(); i++)
    {
        for (int j=0; j<pitchList[i]->size(); j++)
        {
            Logger::outputDebugString("Pitch["+String(i)+","+String(j)+"]: "+ String(pitchList[i]->getUnchecked(j)));
        }
    }
}


void TapGenerator::updateInputTapper(MidiBuffer &midiMessages, Counter globalCounter)
{
    // if the block has MIDI events in it
    // and a note hasn't already been registered in this window...
    if(!midiMessages.isEmpty() /*&& !userInputDetected*/)
    {
        // iterate through the events...
        MidiBuffer::Iterator messages(midiMessages);
        MidiMessage result;
        int samplePos;
        
        // get all of the midi messages in the buffer...
        while(messages.getNextEvent(result, samplePos))
        {
            if(
               result.isNoteOn() && // if the incoming event is a noteOn
               inputTapper.getChannel() == result.getChannel() && // and it is from this channel
               !userInputDetected // and no other noteOn event has been logged...
               )
            {
                // update pitch of input tapper...
                int currentEventNum = inputTapper.numberOfNoteOffs.inSamples();
                if(currentEventNum < pitchList[0]->size())
                    inputTapper.setFreq(pitchList[0]->getUnchecked(currentEventNum));
                
                // clear noteOn and replace with new pitch here...
                
                inputTapper.turnNoteOn(midiMessages, samplePos, globalCounter, false);
                userInputDetected = true; // tell the tapManager that a noteOn has been registered.
                numberOfInputTaps.iterate(); // count the number of taps that have been logged
            }
            else if(
                    result.isNoteOff() && // if the incoming event is a noteOff
                    inputTapper.getChannel() == result.getChannel() // and it is from this channel
                    // but we don't care if a note has already been logged as this may cause noteOffs to be missed when the unserInputDetected flag is reset!
                    )
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

// this function just uses noise to humanise the samples...
void TapGenerator::transformNoise(int randWindowMs)
{
    // old implementation: just uses randomness...
     int randWinInSamples = (randWindowMs/1000.0)*fs;  // amount of randomness to be added
    
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


void TapGenerator::transformLPC()
{
    // Todo------
    // what happens when input does not happen?
    // load alpha from csv file

    // copy everything into local arrays so the input and synths are in the same loop?
    Array<double> t, Mprev, sigmaM, sigmaT, /* onset times and noise vars */
                  Tn, Mn, Hn;               /* intermediate vars */
    
    double scaledRandomValue;
    
    // first add the input tapper...
    t.add(inputTapper.getOnsetTime());                // ...already in samples
    Mprev.add(inputTapper.MNoisePrevValue);           // ...already in samples
    sigmaM.add( (inputTapper.MNoiseStd/1000)  * fs ); // ...converted from ms to samples
    sigmaT.add( (inputTapper.TKNoiseStd/1000) * fs ); // ...converted from ms to samples
    
    //then synth Tappers...
    for (int tapper=0; tapper<numSynthesizedTappers; tapper++)
    {
        t.add(synthesizedTappers[tapper]->getOnsetTime());
        Mprev.add(synthesizedTappers[tapper]->MNoisePrevValue);
        sigmaM.add( (synthesizedTappers[tapper]->MNoiseStd/1000)*fs);
        sigmaT.add( (synthesizedTappers[tapper]->TKNoiseStd/1000)*fs);
    }

    // calculate intermediate noise vars...
    for (int tapper=0; tapper<t.size(); tapper++)
    {
        scaledRandomValue = (rand.nextDouble() -.5)*2;
        Tn.add(TKInterval + sigmaT[tapper] * scaledRandomValue); // THIS SHOULD BE GAUSSIAN -/+ 1
        
        scaledRandomValue = (rand.nextDouble() -.5)*2;
        Mn.add(sigmaM[tapper] * scaledRandomValue);
        Hn.add(Tn[tapper]+Mn[tapper] - Mprev[tapper]);
        
        // update prev motor noise values....
        if (tapper==0)
            inputTapper.MNoisePrevValue = Mn[tapper];
        else
            synthesizedTappers[tapper-1]->MNoisePrevValue = Mn[tapper];
    }

    // make a matrix of Asynchronies...
    for (int i=0; i<t.size(); i++)
    {
        for (int j=0; j<t.size(); j++)
        {
            int tempAsync = t[j] - t[i];
            asynch[i]->set(j, tempAsync);
            asynchAlpha[i]->set(j, tempAsync * alpha[i]->getUnchecked(j));
        }
    }
    
    // generate the next onset times using the sum of the noise and alphaAsyncs for each tapper...
    for (int tapper=1; tapper<t.size(); tapper++)
    {
        // sum the current column of the alphaAsync matrix...
        double sumAlphAsyncColumn = 0;
        for (int i=0; i<t.size(); i++)
            sumAlphAsyncColumn += asynchAlpha[i]->getUnchecked(tapper);
        // add it to the current onset and the noise and update the corresponding synth Tapper...
//        Logger::outputDebugString(String(tapper)+", AsyncColSum: "+String(sumAlphAsyncColumn)+", Hn: "+String(Hn[tapper]));
        synthesizedTappers[tapper-1]->setInterval(sumAlphAsyncColumn + Hn[tapper] /*+t[tapper]*/);
//        Logger::outputDebugString("SynthTapper["+String(tapper-1)+"] interval: "+String(synthesizedTappers[tapper-1]->getInterval()));
    }
}


void TapGenerator::logResults(String inputString)
{
    // write to terminal
    Logger::outputDebugString(inputString);
    
    String inputOnsetTime = "NaN ";
    
    if (userInputDetected)
    {
        inputOnsetTime = String(inputTapper.getOnsetTime());
    }
    
    captainsLog->writeText(inputOnsetTime, false, false);
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        captainsLog->writeText(" "+String(synthesizedTappers[i]->getOnsetTime()), false, false);
    }
    captainsLog->writeText(";\n", false, false);
}


void TapGenerator::updateTappersPitch(int tapperNum)
{
    int numIntroBeeps = 4;
    
    // the pitch of the tapper is determined by the number of NoteOffs
    // this means a note off has to happen before the next noteOn or the pitch will not be updated
    int currentEventNum = synthesizedTappers[tapperNum]->numberOfNoteOffs.inSamples();
    
    // if the current event number is within the number of available pitches
    if(currentEventNum < numIntroBeeps)
    {
        // intro - beeps at the same freq...
        synthesizedTappers[tapperNum]->setFreq(84);
    }
    else if(currentEventNum > pitchList[tapperNum+1]->size())
    {
        // turn the tappers down if the MIDI file runs out...
        synthesizedTappers[tapperNum]->setFreq(84);
        synthesizedTappers[tapperNum]->setVel(0);
    }
    else
    {
        synthesizedTappers[tapperNum]->setFreq(pitchList[tapperNum+1]->getUnchecked(currentEventNum-numIntroBeeps));
    }
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
void TapGenerator::nextBlock(MidiBuffer &midiMessages, Counter &globalCounter, int blockSize)
{
    
    // update block size from process block...
    frameLen = blockSize;
    
    // update the input tapper with incoming on/off messages...
    updateInputTapper(midiMessages, globalCounter);

    // GLOBAL SAMPLE COUNTER LOOP --------------------------------------
    for (int sampleNum=0; sampleNum<frameLen; sampleNum++)
    {
        // iterate the synthesized tappers...
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)
        {
            updateTappersPitch(tapperNum);
            synthesizedTappers[tapperNum]->iterate(midiMessages, sampleNum, globalCounter, notesTriggered);
        }
        
        // BEAT COUNTER -------------------------------------------
        if(allNotesHaveBeenTriggered())
        {
            if(userInputDetected)
            {
                // Recalculate timing params with all registered asynch values...
                //transformNoise(10.0); //input = ms windows
                transformLPC();
                
                logResults("Beat ["+String(beatCounter.inSamples())+"], user input ["+String(numberOfInputTaps.inSamples())+"] found");
                beatCounter.iterate(); // count the number of registered beats
                updateTapAcceptanceWindow();
                resetTriggeredFlags();
                userInputDetected = false;
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
                    userInputDetected = false; // this should already be false!
                }
                else
                {
                    // Counting towards the next beat threshold...
                    // ...this could be used to feed params back to the UI
                }
            }
            
//            // write out the tapper intervals to see when they are supposed to next tap...
//            for (int tapper=0; tapper<numSynthesizedTappers; tapper++)
//            {
//                Logger::outputDebugString("SynthTapper["+String(tapper)+"] interval: "+String(synthesizedTappers[tapper]->getInterval()));
//            }
//            Logger::outputDebugString("\n");
        }
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

//
//  TapManager.cpp
//  Metro
//
//  Created by Ryan Stables on 24/02/2017.
//
//
#include "TapManager.hpp"


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
    
    File logFile = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("logFile.txt");
    
//    File logFile (localDataPath+"savedData.m");
    
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
        {  0,   0,  0,  0},
        {0.25,   0,  0,  0},
        {0.25,   0,  0,  0},
        {0.25,   0,  0,  0}
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
        int pitch = 60+(i*12);
        synthesizedTappers[i]->updateParameters(i+1 /*ID*/, i+2 /*channel*/, pitch /*freq*/, 22050 /*noteLen*/, 44100 /*interval*/, 127 /*velocity*/);
    }
    
    // initialise pitch list...
    readPitchListFromPreloadedArray();

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
    // reset the tappers...
    for (int tapper=0; tapper<numSynthesizedTappers; tapper++)
        synthesizedTappers[tapper]->reset();
    // reset theTKinterval etc...
    updateBPM(bpm);
    // reset all of the counters...
    beatCounter.reset();
    numberOfInputTaps.reset();
    resetTriggeredFlags();    
    // this is an absolute value, so it gets reset based on the BPM...
    nextWindowThreshold=TKInterval*1.5;
    // Write results to the the end of the file...
    trialNum.iterate();
    captainsLog->writeText("];\n\n% Trial: "+String(trialNum.inSamples())+"\nx_"+String(trialNum.inSamples())+"=[\n", false, false);
    captainsLog->flush();
}

// generate a random value and scale it by the chosen mean and std dev...
double TapGenerator::getRandomValue(double std)
{
    // make the random var -1/1...
    double r = rand.nextDouble()*2-1;
    return r*std;
}

void TapGenerator::readPitchListFromMidiSeq(const OwnedArray<MidiMessageSequence> &inputMIDISeq)
{
    // remove what was already in the list...
    pitchList.clear();
    
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
                int channel = tempEventHolder->message.getChannel();
                Logger::outputDebugString(String(channel)+", "+String(pitch)+";");
                pitchList[trackNum]->add(pitch);
            }
        }
    }
}

void TapGenerator::readPitchListFromPreloadedArray()
{
    // remove anything currently in the pitch List
    pitchList.clear();
    
    for (int trackNum=0; trackNum < preloadedHaydn.numChannels; trackNum++)
    {
        pitchList.add(new Array<double>);
        for (int eventNum=0; eventNum<preloadedHaydn.numEventsPerChannel; eventNum++)
        {
            int pitch = preloadedHaydn.pitchList[trackNum][eventNum];
            pitchList[trackNum]->add(pitch);
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


void TapGenerator::updateInputTapper(MidiBuffer &midiMessages, Counter globalCounter, int beatNumber)
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
                
                inputTapper.turnNoteOn(midiMessages, samplePos, globalCounter, beatNumber, false);
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
            prevMean+=(synthesizedTappers[i]->getPrevOnsetTime()/(double)numSynthesizedTappers);
        }
        
        inputTapAcceptanceWindow = currentMean-prevMean;
//        nextWindowThreshold = currentMean + inputTapAcceptanceWindow / 2;
//        Logger::outputDebugString("Next Thresh: "+String(nextWindowThreshold)+"\n");
    }
    else
    {
        // this should only happen on the first beat.
        nextWindowThreshold = TKInterval*1.5;
//        Logger::outputDebugString("Next Thresh: "+String(nextWindowThreshold)+"\n");
    }
}

double TapGenerator::meanSynthesizedOnsetTime()
{
    double currentMean=0;
    for (int i=0; i<numSynthesizedTappers; i++)
        currentMean+=(synthesizedTappers[i]->getOnsetTime()/(double)numSynthesizedTappers);
    return currentMean;
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
 
    // DEBUG ---------------------
    for (int i=0; i<4; i++)
        Logger::outputDebugString(String(alpha[i]->getUnchecked(0))+", "+String(alpha[i]->getUnchecked(1))+", "+String(alpha[i]->getUnchecked(2))+", "+String(alpha[i]->getUnchecked(3)));
    // DEBUG ---------------------
    
    Array<double> t, sigmaM, sigmaT,     /* onset times and noise params */
                  TkNoise, MotorNoise, MotorNoisePrev, Hnoise;  /* noise vars */
    
    t.add(inputTapper.getOnsetTime()); // ...first add the input tapper
    MotorNoisePrev.add(inputTapper.MNoisePrevValue);
    sigmaM.add(inputTapper.getMNoiseStdInSamples(fs));
    sigmaT.add(inputTapper.getTKNoiseStdInSamples(fs));
    for (int tapper=0; tapper<numSynthesizedTappers; tapper++) // ...then the synth tappers
    {
        t.add(synthesizedTappers[tapper]->getOnsetTime());
        MotorNoisePrev.add(synthesizedTappers[tapper]->MNoisePrevValue);
        sigmaM.add(synthesizedTappers[tapper]->getMNoiseStdInSamples(fs));
        sigmaT.add(synthesizedTappers[tapper]->getTKNoiseStdInSamples(fs));
    }
  
    // calculate noise vars for all tappers...
    for (int i=0; i<t.size(); i++)
    {
        TkNoise.add(getRandomValue(sigmaT[i]));
        MotorNoise.add(getRandomValue(sigmaM[i]));
        Hnoise.add(TkNoise[i] + MotorNoise[i] - MotorNoisePrev[i]);

        
        double sumAsync = 0;
        for (int j=0; j<t.size(); j++)
        {
            asynch[i]->set(j, t[i] - t[j]);
            asynchAlpha[i]->set(j, alpha[i]->getUnchecked(j) * asynch[i]->getUnchecked(j));
            sumAsync += asynchAlpha[i]->getUnchecked(j);
        }
        Logger::outputDebugString(String(sumAsync));
        
        if (i==0)
        {   // update input motor noise of input tapper...
            inputTapper.MNoisePrevValue = MotorNoise[i];
        }
        else
        {   // update the next tap time for tapper i...
            synthesizedTappers[i-1]->setInterval(TKInterval - sumAsync + Hnoise[i]);
            
            int onsetTime   = t[i],
                noise       = Hnoise[i],
                nextOnset   = onsetTime + TKInterval - sumAsync + noise;

            synthesizedTappers[i-1]->setNextOnsetTime(nextOnset); // to be replaced with "nextOnset"
            // synthesizedTappers[i-1]->setNextOnsetTime(onsetTime + TKInterval); // ...debugging
            
            // update motor noise...
            synthesizedTappers[i-1]->MNoisePrevValue = MotorNoise[i];
        }
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
    updateInputTapper(midiMessages, globalCounter, beatCounter.inSamples());
    
    // GLOBAL SAMPLE COUNTER LOOP --------------------------------------
    for (int sampleNum=0; sampleNum<frameLen; sampleNum++)
    {
        // iterate the synthesized tappers...
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)
        {
            updateTappersPitch(tapperNum);
            synthesizedTappers[tapperNum]->iterate(midiMessages, sampleNum, globalCounter, beatCounter.inSamples(), notesTriggered);
        }
        
        // BEAT COUNTER -------------------------------------------
        if(allNotesHaveBeenTriggered()) // ...if all synthesized tappers have tapped
        {
            if(userInputDetected) // ...and the user has tapped too
            {
                // apply transformation...
                transformLPC();
                
                logResults("Beat ["+String(beatCounter.inSamples())+"], user input ["+String(numberOfInputTaps.inSamples())+"] found");
                beatCounter.iterate(); // count the number of registered beats
                updateTapAcceptanceWindow();
                resetTriggeredFlags();

                Logger::outputDebugString("-----------------------------");

            }
            else // ...if no user input has happened yet...
            {
                nextWindowThreshold = meanSynthesizedOnsetTime()+inputTapAcceptanceWindow/2;

                if(globalCounter.inSamples() >= nextWindowThreshold) // ...if the end of the current beatWindow is reached
                {
                    for (int synthTapper=0; synthTapper<numSynthesizedTappers; synthTapper++)
                    {
                        int lastTapTime = synthesizedTappers[synthTapper]->getOnsetTime();
                        int interval = synthesizedTappers[synthTapper]->getInterval();
                        synthesizedTappers[synthTapper]->setNextOnsetTime( lastTapTime + interval);
                    }
                        
                    // Recalculate timing params without the user input asnynch...
                    logResults("Beat ["+String(beatCounter.inSamples())+"] threshold reached");
                    updateTapAcceptanceWindow(); //<--- calculate the next window thresh here
                    resetTriggeredFlags();
                    beatCounter.iterate(); // count the beats

                    Logger::outputDebugString("-----------------------------");
                }
                else
                {
                    // Counting towards the next beat threshold...
                    // ...this could be used to feed params back to the UI
                }
            }
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

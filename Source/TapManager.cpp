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
    time = time.getCurrentTime();
    
    File logFile = File::getSpecialLocation(File::userDocumentsDirectory)
    .getChildFile("Log_"+
                  String(time.getHours()) + "-" +
                  String(time.getMinutes()) + "-" +
                  String(time.getSeconds()) + "_" +
                  String(time.getDayOfMonth()) +
                  time.getMonthName(true) +
                  String(time.getYear()) + ".csv");
    
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
    logFile.appendText("N, P1 (input), P2, P3, P4, , P1 Int, P2 Int, P3 Int, P4 Int, , P1 MVar, P2 MVar, P3 MVar, P4 MVar, ,P1 TKVar, P2 TKVar, P3 TKVar, P4 TKVar, , Async 11, Async 12, Async 13, Async 14, Async 21, Async 22, Async 23, Async 24, Async 31, Async 32, Async 33, Async 34, Async 41, Async 42, Async 43, Async 44, , Alpha 11, Alpha 12, Alpha 13, Alpha 14, Alpha 21, Alpha 22, Alpha 23, Alpha 24, Alpha 31, Alpha 32, Alpha 33, Alpha 34, Alpha 41, Alpha 42, Alpha 43, Alpha 44, , P1 TKStd, P2 TKStd, P3 TKStd, P4 TKStd, , P1 MStd, P2 MStd, P3 MStd, P4 MStd, , P1 Vol, P2 Vol, P3 Vol, P4 Vol \n");
    captainsLog = new FileOutputStream (logFile);
}

TapGenerator::~TapGenerator()
{
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
    inputTapper.numberOfNoteOffs.reset();
    resetTriggeredFlags();    
    // this is an absolute value, so it gets reset based on the BPM...
    nextWindowThreshold=TKInterval*1.5;
    // Write results to the the end of the file...
    trialNum.iterate();
    captainsLog->writeString(" , , , \n");
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
    // ------ update the pitch of the input tapper -------
    int currentEventNum = inputTapper.numberOfNoteOffs.inSamples();
    
    // ------ if the block has MIDI events in it ------
    if(!midiMessages.isEmpty() /*&& !userInputDetected*/)
    {
        // ------- iterate through the events in the Midi Buffer ------
        MidiBuffer newMidiBuffer;
        MidiBuffer::Iterator messages(midiMessages);
        MidiMessage result;
        int samplePos;
        while(messages.getNextEvent(result, samplePos))
        {
            // ------ send pitch and vol values from inputTapper to MidiBuffer-------
            if(result.isNoteOnOrOff()) {
                if(currentEventNum < pitchList[0]->size())
                {
                    Logger::outputDebugString("CurrEvent: " + String(currentEventNum) + ", beatNum: " + String(beatNumber) + ", max: " + String(pitchList[0]->size()));
                    
                    int num = pitchList[0]->getUnchecked(currentEventNum);
                    inputTapper.setFreq(num);
                }
                result.setVelocity(inputTapper.getVel() / 128.f);
                result.setNoteNumber(inputTapper.getFreq());
                newMidiBuffer.addEvent(result, samplePos);
            }
            
            // ------ If noteOn from current channel happens -------
            if(
               result.isNoteOn() && // if the incoming event is a noteOn
               inputTapper.getChannel() == result.getChannel() && // and it is from this channel
               !userInputDetected // and no other noteOn event has been logged during this window...
            )
            {
                inputTapper.turnNoteOn(midiMessages, samplePos, globalCounter, beatNumber, false);
                userInputDetected = true; // tell the tapManager that a noteOn has been registered.
                numberOfInputTaps.iterate(); // count the number of taps that have been logged
            }
            // ------ If noteOff from current channel happens -------
            else if(
                    result.isNoteOff() && // if the incoming event is a noteOff
                    inputTapper.getChannel() == result.getChannel() // and it is from this channel
                    // but we don't care if a note has already been logged as this may cause noteOffs to be missed when the unserInputDetected flag is reset!
                    )
            {
                inputTapper.turnNoteOff(midiMessages, samplePos, globalCounter, false);
            }
        }
        midiMessages = newMidiBuffer;
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
    }
    else
    {
        // this should only happen on the first beat.
        nextWindowThreshold = TKInterval*1.5;
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
    Array<double> t, sigmaM, sigmaT, MotorNoisePrev,    /* onset times and noise params */
                  TkNoise, MotorNoise, Hnoise;          /* new noise vars */
    TKNoiseStr = MNoiseStr = alphaStr = asyncStr = TKNParamStr = MNParamStr = volStr = ""; // reset logFile strings
    
    
    //  ---------- Add params for the first tapper  ----------
    if(userInputDetected) {
        t.add(inputTapper.getOnsetTime()); // ...first add the input tapper
        MotorNoisePrev.add(inputTapper.MNoisePrevValue);
        sigmaM.add(inputTapper.getMNoiseStdInSamples(fs));
        sigmaT.add(inputTapper.getTKNoiseStdInSamples(fs));
    } else {
        // if no input tap happens, we can still apply a transformation to the synthesized
        // tappers by cancelling out the asynchrony of the input tapper...
        t.add(inputTapper.getPrevOnsetTime() + inputTapper.getInterval());
        MotorNoisePrev.add(0.f);
        sigmaM.add(0.f);
        sigmaT.add(0.f);
    }

    //  ---------- Update params for the syntehsized tappers  ----------
    for (int tapper=0; tapper<numSynthesizedTappers; tapper++) // ...then the synth tappers
    {
        t.add(synthesizedTappers[tapper]->getOnsetTime());
        MotorNoisePrev.add(synthesizedTappers[tapper]->MNoisePrevValue);
        sigmaM.add(synthesizedTappers[tapper]->getMNoiseStdInSamples(fs));
        sigmaT.add(synthesizedTappers[tapper]->getTKNoiseStdInSamples(fs));
    }
  

    //  ---------- apply the actual transformation  ----------
    for (int i=0; i<t.size(); i++)
    {
        // ---------- update the noise params for each tapper ----------
        TkNoise.add(getRandomValue(sigmaT[i]));
        MotorNoise.add(getRandomValue(sigmaM[i]));
        Hnoise.add(TkNoise[i] + MotorNoise[i] - MotorNoisePrev[i]);
        
        //  ---------- Update logfile strings  ----------
        double vol = (i == 0) ? double(inputTapper.getVel()) : double(synthesizedTappers[i-1] -> getVel());
        TKNoiseStr = TKNoiseStr + String(TkNoise[i] / fs) + ", ";
        MNoiseStr = MNoiseStr + String(MotorNoise[i] / fs) + ", ";
        TKNParamStr = TKNParamStr + String(sigmaT[i] / fs) + ", ";
        MNParamStr = MNParamStr + String(sigmaM[i] / fs) + ", ";
        volStr = volStr + String( vol / 128.f) + ", ";
        
        // ---------- apply the LPC model ----------
        double sumAsync = 0;
        for (int j=0; j<t.size(); j++)
        {
            if (j==0 && !userInputDetected)
            {   // no input player asynch if tap didn't happen.
                asynch[i]->set(j, 0);
                asyncStr = asyncStr + "0, ";
            } else {
                asynch[i]->set(j, t[i] - t[j]);
                asyncStr = asyncStr + String((t[i] - t[j]) / fs) + ", ";
            }
            alphaStr = alphaStr + String(alpha[i]->getUnchecked(j)) + ", ";
            asynchAlpha[i]->set(j, alpha[i]->getUnchecked(j) * asynch[i]->getUnchecked(j));
            sumAsync += asynchAlpha[i]->getUnchecked(j);
        }
        if (i==0)
        {   // update input motor noise of input tapper...
            inputTapper.MNoisePrevValue = MotorNoise[i];
            inputTapper.setInterval(inputTapper.getOnsetTime() - inputTapper.getPrevOnsetTime());
        }
        else
        {   // update the next tap time for tapper i...
            synthesizedTappers[i-1]->setInterval(TKInterval - sumAsync + Hnoise[i]);
            int nextOnset   = t[i] + TKInterval - sumAsync + Hnoise[i];
            synthesizedTappers[i-1]->setNextOnsetTime(nextOnset);
            // update motor noise...
            synthesizedTappers[i-1]->MNoisePrevValue = MotorNoise[i];
        }
    }
}

void TapGenerator::logResults(String inputString)
{
    String onsetTimes = userInputDetected ? String(inputTapper.getOnsetTime() / fs)+", " : "NaN, ";
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        onsetTimes = onsetTimes + String(synthesizedTappers[i]->getOnsetTime() / fs) + ", ";
    }    
    String intervals;
    if(beatCounter.inSamples() > 0) {
        
        if(inputTapper.thisEventWasTriggeredByHuman() && inputTapper.prevEventWasTriggeredByHuman()) {
            intervals = String(inputTapper.getInterval() / fs) + ", ";
        } else {
            intervals = "NaN, ";
        }
        for (int i=0; i<numSynthesizedTappers; i++)
        {
            intervals = intervals + String(synthesizedTappers[i]->getInterval() / fs) + ", ";
        }
    } else {
        intervals = "NaN, NaN, NaN, NaN, ";
    }
    String outputLine = String(beatCounter.inSamples()) + ", " + onsetTimes + ", " + intervals + ", " + MNoiseStr + ", " + TKNoiseStr + ", " + asyncStr + ", " + alphaStr + ", " + TKNParamStr + ", " + MNParamStr + ", " + volStr + "\n";

    // Debugging ---------
    Logger::outputDebugString(inputString);
    // Debugging ---------
    captainsLog->writeString(outputLine);
}

void TapGenerator::updateTappersPitch(int tapperNum)
{
    // the pitch of the tapper is determined by the number of NoteOffs
    // this means a note off has to happen before the next noteOn or the pitch will not be updated
    int currentEventNum = synthesizedTappers[tapperNum]->numberOfNoteOffs.inSamples();
    
    // if the current event number is within the number of available pitches
    if(currentEventNum < numIntroBeeps)
    {
        // intro - beeps at the same freq...
        synthesizedTappers[tapperNum]->setFreq(84);
    }
    else if(currentEventNum >= pitchList[tapperNum+1]->size() + numIntroBeeps)
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
            synthesizedTappers[tapperNum] -> iterate(midiMessages, sampleNum, globalCounter, beatCounter.inSamples(), notesTriggered);
        }
        
        // BEAT COUNTER -------------------------------------------
        if(allNotesHaveBeenTriggered()) // ...if all synthesized tappers have tapped
        {
            if(userInputDetected) // ...and the user has tapped too
            {
                // apply transformation...
                transformLPC();
                inputTapper.setTriggeredByHuman(true); // <- register that input tap has happened
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
                    // set next onsets using phase correction model...
                    transformLPC();
                    inputTapper.setTriggeredByHuman(false); // <--- register that a tap never happened
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
//        Tapper tempTapper = synthesizedTappers[i];
        if(synthesizedTappers[i]->isActive())
        {
            synthesizedTappers[i]->kill(midiMessages);
        }
    }
}

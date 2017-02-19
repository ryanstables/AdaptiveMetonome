//
//  TapGenerator.cpp
//  adaptiveTappers
//
//  Created by Ryan Stables on 18/02/2017.
//

#include "TapGenerator.h"

// -------------------------------------------
// Tapper functions -------------------------
// -------------------------------------------
Tapper::Tapper()
{
    TKNoiseMS = 25; //25 ms timekeeper noise
    MNoise    = 10; //10 ms Motor noise
}

Tapper::~Tapper()
{

}

void Tapper::noteOn(MidiBuffer &midiMessages, int sampleNo)
{
//    // why can't i use the MidiMessage::noteOn function???
//    midiMessages.addEvent(MidiMessage::noteOn (MIDIChannel, tapperFreq, tapperVel, sampleNo);
}

void Tapper::noteOff(MidiBuffer &midiMessages, int sampleNo)
{
//    // why can't i use the MidiMessage::noteOff function???
//    midiMessages.addEvent(MidiMessage::noteOff (MIDIChannel, tapperFreq, tapperVel, sampleNo);
}



// -------------------------------------------
// Tap generator functions -------------------
// -------------------------------------------
TapGenerator::TapGenerator(int NumTappers)
{
    // allocate tappers...
    nTappers = NumTappers;
    tappers = new Tapper[nTappers];
    Hn      = new float[nTappers];
    Mn      = new float[nTappers];
    Tn      = new float[nTappers];
    Mprev   = new float[nTappers];
    sigmaM  = new float[nTappers];
    sigmaT  = new float[nTappers];
    t       = new float[nTappers];
    alpha   = new float*[nTappers];
    A   = new float*[nTappers];
    for (int i=0; i<nTappers; i++)
    {
        alpha[i] = new float[nTappers];
        A[i]     = new float[nTappers];
    }
    
    // preset the LPC parameters...
    TKInterval = static_cast <int> (floor(60.f/bpm*fs));
    for (int i=0; i<nTappers; i++)
    {
        t[i] =0;
    }
}

TapGenerator::~TapGenerator()
{
    // free vectors...
    delete[] tappers;
    delete[] Hn;
    delete[] Mn;
    delete[] Tn;
    delete[] Mprev;
    delete[] sigmaM;
    delete[] sigmaT;
    delete[] t;
    // free matrices...
    for (int i=0; i<nTappers; i++)
    {
        delete[] alpha[i];
        delete[] A[i];
    }
    delete[] alpha;
    delete[] A;
}

void TapGenerator::next(juce::MidiBuffer &midiMessages, int sampleNum)
{
//    bpm = playhead.bpm;                                                                 // get the BPM from the playhead (if it updates)
    

    /*
    // init the first beat to the interval (this can't be done in the prepareToPlay() fn?)
    if(!beatCounter)
    {
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)                  // loop to synthesize the tappers
            synthesizedTapperInterval[tapperNum] = interval;
    }
    
    for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)                  // loop to synthesize the tappers
    {
        int synthTapperNum  = tapperNum+1;                                              // set MIDI channels for each tapper
        int channelNum      = tapperNum+2;                                              // set arrayPos for each tapper
        // ...input is on channel 1, so first synth tapper is channel 2
        synthesizedTapperNoteLen[tapperNum] = round(interval/ *DurParam[tapperNum]);    // note lenth calculated as a beat division (in samples)
        
        for(int sampleNo=0; sampleNo<frameLen; sampleNo++)                              // loop over the samples in the current block...
        {
            //set NoteOn Messages --------
            int noteOnThresh = synthesizedTapperInterval[tapperNum]-1;
            if(noteOnCounter[tapperNum]<noteOnThresh)                                   // add noteOn events if the counter reaches the interval...
            {
                noteOnCounter[tapperNum]++;                                             // increment current noteOn counter
            }
            else
            {                                                                                       // NoteOn message at the current samplePosition...
                //apply transformation to the current interval...
                double asynchrony = currentTapTimes[synthTapperNum] - meanTapTime;
                synthesizedTapperInterval[tapperNum]   = transformIOI(interval, asynchrony);                    // add pertubation to the intervals
                
                if(synthesizedTapperNoteLen[tapperNum] >= synthesizedTapperInterval[tapperNum])     // to stop the note length exceeding the interval
                {
                    synthesizedTapperNoteLen[tapperNum] = synthesizedTapperInterval[tapperNum]-1;   // ...this shouldn't happen, but could if pertubations are high
                    Logger::outputDebugString("% Warning: interval shorter than note length on channel "+String(channelNum));
                }                                                                                   // ...as the lengths are based on the raw (not transformed) interval
                
                
                midiMessages.addEvent(MidiMessage::noteOn (channelNum, *FreqParam[tapperNum], static_cast <uint8> (*VelParam[tapperNum])),sampleNo);
                noteCurrentlyActive[tapperNum]=true;                                    // set noteActive flag so it can be removed when plyhead stops
                
                //register the time stamp and update the old one...
                prevTapTimes[synthTapperNum] = currentTapTimes[synthTapperNum];
                currentTapTimes[synthTapperNum] = ((frameLen * frameCounter) + sampleNo);   //onset time in samples (x/fs for seconds)
                channelHasNewValue[tapperNum] = true; //only has values for the synth tappers so far?
                
                noteOnCounter[tapperNum] = 0;                                               // reset notOn counter
                noteCounter++;                                                              // iterate note counter
            }
            
            //set NoteOff Messages --------
            int noteOffThresh = synthesizedTapperInterval[tapperNum]+synthesizedTapperNoteLen[tapperNum]-1;
            if(noteOffCounter[tapperNum]<=noteOffThresh)
            {
                noteOffCounter[tapperNum]++;                                            // increment noteOff counter if
            }
            else
            {
                midiMessages.addEvent(MidiMessage::allNotesOff(channelNum), sampleNo);          // add note off when the counter reaches the interval + noteLen...
                currentNoteOffTimes[synthTapperNum] = ((frameLen * frameCounter) + sampleNo);   //offset time in samples (x/fs for seconds)
                
                noteOffCounter[tapperNum] = synthesizedTapperNoteLen[tapperNum];                // reset noteOff counter to the noteLen
                noteCurrentlyActive[tapperNum]=false;                                           // cancel noteActive flag
            }
        }
    }
     */
}

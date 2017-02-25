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
    TKNoiseMS = 25;  //25 ms timekeeper noise
    MNoise    = 10;  //10 ms Motor noise
    setNoteLen(256);
    setVel(127);
    setFreq(60); // in MidiNoteNums
}

Tapper::~Tapper()
{
    
}

void Tapper::turnNoteOn(MidiBuffer &midiMessages, int sampleNo)
{
    midiMessages.addEvent(MidiMessage::noteOn (MIDIChannel, tapperFreq, (uint8)tapperVel), sampleNo);
    noteActive=true;
    noteNumber++;
}

void Tapper::turnNoteOff(MidiBuffer &midiMessages, int sampleNo)
{
    midiMessages.addEvent(MidiMessage::noteOff (MIDIChannel, tapperFreq, (uint8)tapperVel), sampleNo);
    noteActive=false;
    resetOffsetCounter();

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
    if(inputCounter.inSamples()%interval==0 && /*!noteActive &&*/ inputCounter.inSamples())
        return true;
    else
        return false;
}

// check to see if the noteOff message needs to be triggered...
bool Tapper::requiresNoteOff()
{
    if (offseCounter.inSamples()>=noteLen)
        return true;
    else
        return false;
}

void Tapper::iterateCounter(MidiBuffer & midiMessages, int sampleNum, Counter &globalCounter)
{
    // check to see if noteOns/Offs need to be added....
    if(requiresNoteOn(globalCounter))
    {
        Logger::outputDebugString("NoteON: "+String(globalCounter.inSamples()));
        turnNoteOn(midiMessages, sampleNum);
    }
    else if(requiresNoteOff())
    {
        turnNoteOff(midiMessages, sampleNum);
        Logger::outputDebugString("NoteOff: "+String(globalCounter.inSamples()));
    }
    
    // are these iterated properly here? i.e. never hits 0, alwys starts at sample 1?
    globalCounter.iterate();
    
    if(noteActive) // if note is turned on, start counting towards the noteOff...
        offseCounter.iterate();
}




//==============================================================================
//========= Generator ==========================================================
//==============================================================================
TapGenerator::TapGenerator(int NumTappers, double sampleRate, int samplesPerBlock)
{
    // store number of tappers...
    numSynthesizedTappers = NumTappers;
    fs = sampleRate;
    frameLen = samplesPerBlock;
    
    
    // allocate the tappers...
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        tappers.add(new Tapper);
    }

    // ????
    for(int i=0;i<tappers.size(); i++)
    {
        tappers[i]->setID(i);
        tappers[i]->setChannel(i+2); // the first one is the input tapper, start from chan 2
        
        // all this will be altered by the host/user...
        tappers[i]->setFreq(60+(i*12)); //octaves above middle C
        tappers[i]->setNoteLen(22050);
        tappers[i]->setInterval(44100);
        tappers[i]->setVel(127);
    }
}


TapGenerator::~TapGenerator()
{
}

void TapGenerator::initBPM(double x)
{
    bpm = x;
    TKInterval = static_cast <int> (floor(60.f/bpm*fs));
    for (int i; i<numSynthesizedTappers; i++)
    {
        tappers[i]->setInterval(TKInterval); //plays constantly?!?
        tappers[i]->setNoteLen(TKInterval/2);
    }
};


// run in each process bock to update the note on/offs...
void TapGenerator::nextBlock(MidiBuffer &midiMessages, Counter &globalCounter)
{
    
    // loop over the input buffer...
    for (int sampleNum=0; sampleNum<frameLen; sampleNum++)
    {
        // loop over the synthesized tappers...
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)
        {
            // TODO: if allNotesHappened(), then recalculateIntervals();
            tappers[tapperNum]->iterateCounter(midiMessages, sampleNum, globalCounter);
            
        }
    }
    
}


// Kill the taps that get left on if the playhead stops whilst a note is active...
void TapGenerator::killActiveTappers(MidiBuffer &midiMessages)
{
    for (int i=0; i<numSynthesizedTappers; i++)
    {
        if(tappers[i]->isActive())
        {
            tappers[i]->kill(midiMessages);
        }
    }
}

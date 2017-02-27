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
    setFreq(60); // in MidiNotes
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
    if(inputCounter.inSamples()%interval==0 && !noteActive && inputCounter.inSamples())
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
        Logger::outputDebugString("ID: "+String(tapperID)+", Channel: "+String(MIDIChannel)+", NoteON: "+String(globalCounter.inSamples()));
        turnNoteOn(midiMessages, sampleNum);
        prevOnsetTime.set(globalCounter.inSamples());
        notesTriggered[tapperID] = true;
    }
    else if(requiresNoteOff())
    {
        turnNoteOff(midiMessages, sampleNum);
        Logger::outputDebugString("ID: "+String(tapperID)+", Channel: "+String(MIDIChannel)+",  NoteOff: "+String(globalCounter.inSamples()));
    }
    
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

    // init the tappers. Maybe move this to an updateTapperParams() fn because it needs to be updated in the processBlock too. 
    for(int i=0; i<numSynthesizedTappers; i++)
    {
        tappers[i]->setID(i);
        tappers[i]->setChannel(i+2); // the first one is the input tapper, start from chan 2
        
        // all this will be altered by the host/user...
        tappers[i]->setFreq(60+(i*12)); //octaves above middle C
        tappers[i]->setNoteLen(22050);
        tappers[i]->setInterval(44100);
        tappers[i]->setVel(127);
        
        notesTriggered.push_back(false);
        prevAsynch.push_back(0);
    }
}


TapGenerator::~TapGenerator()
{
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
        tappers[i]->setInterval(TKInterval);
        tappers[i]->setNoteLen(TKInterval/beatDivision);
    }
};


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
        tappers[i]->setInterval(tappers[i]->getInterval()+randomValue - prevAsynch[i]);
        // update the prevAsynch values...
        prevAsynch[i] = randomValue;
    }
}



// run in each process bock to update the note on/offs...
void TapGenerator::nextBlock(MidiBuffer &midiMessages, Counter &globalCounter)
{
    for (int sampleNum=0; sampleNum<frameLen; sampleNum++)
    {
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)
        {
            tappers[tapperNum]->iterate(midiMessages, sampleNum, globalCounter, notesTriggered);
        }
        
        if(allNotesHaveBeenTriggered())
        {
            // recalculate intervals here once all notes from the current beat have been logged...
            Logger::outputDebugString("Beat: "+String(beatCounter.inSamples()));
            transform(); // add pertubation to the onset times
            for (int i=0; i<numSynthesizedTappers; i++)
                notesTriggered[i]=false; // reset the noteTriggered flags
            beatCounter.iterate(); // count the beats
        }
        globalCounter.iterate();
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

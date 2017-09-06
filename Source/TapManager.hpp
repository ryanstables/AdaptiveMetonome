//
//  TapManager.hpp
//  Metro
//
//  Created by Ryan Stables on 24/02/2017.
//
//

#ifndef TapManager_hpp
#define TapManager_hpp

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>

// ------- Todo
//
// 1 - get the transformLPC() fn working,
// 2 - read input tapper notes from file (overwrite output buffer)
// 3 - fix the repeated entry bug
// 4 - load MIDI file from GUI
// ...
// ...
// ...
//    - the input tapper needs to update the middibuffer now, but do the events in it need to be removed first?
//    - do some checks to make sure the midi file and numtappers is aligned, or just read numTapers from file.
//    - currently, the input tapper has to have a note off before the next note on, or it will not update!?
//    - how can the input/synth pitches be aligned, currently if a note is missed everything is out of sync?

// 1 - implement the LPC model in the transform class
//      - import alphas from csv file?

// 3 - properly configure the TapGenerator::reset() function
// 4 - make a UI, where the file path can be set and the tapper params can be adjusted and record can be turned on/off.
//      - add gain controls for each tapper
// 5 - upgrade the whole system so everything runs off the imported MIDI file (incl. init tappers)
//      - allow assignable input tappers (select channel to tap to)

// ----------
// tapping workflow...
// 0 - procesor has a global counter that gets passed into the tapGenerator
// 1 - tapGenerator uses the counter to call the tappers
// 2 - tapper manages the noteOn and noteOff messages
// Notes:
// if there are more than 1 taps at a detected beat, only the first is chosen.
// the LPC model only gets applied when there is a user input.

//==============================================================================
//========= Counter ============================================================
//==============================================================================
class Counter
{
public:
    Counter(){};
    ~Counter(){};
    // do some other stuff to the object...
    void iterate(){counter++;};
    void reset(){counter = 0;};
    void set(int x){counter = x;};
    //return the counter in different formats...
    int inSamples(){return counter;};
    double   inFrames(int frameLen){return (double)counter/(double)frameLen;};
    double   inSeconds(double fs){return (double)counter/fs;};
    double   inMilliseconds(double fs){return inSeconds(fs)*1000.0;};
    
private:
    int counter=0;
};




//==============================================================================
//========= Tapper  ============================================================
//==============================================================================
class Tapper
{
public:
    Tapper();
    ~Tapper();
    // getters/setters...
    void setNoteLen(int x){noteLen=x;};
    int  getNoteLen(){return noteLen;};
    void setVel(int x){tapperVel=x;};
    int  getVel(){return tapperVel;};
    void setFreq(int x){tapperFreq=x;};
    int  getFreq(){return tapperFreq;};
    bool isActive(){return noteActive;};
    void setID(int x){tapperID=x;};
    int  getID(){return tapperID;};
    void setChannel(int x){MIDIChannel=x;};
    int  getChannel(){return MIDIChannel;};
    
    // for onset transform...
    void setTKNoiseStd(int x){TKNoiseStd=x;};
    int  getTKNoiseStd(){return TKNoiseStd;};
    void setMNoiseStd(int x){MNoiseStd=x;};
    int  getMNoiseStd(){return MNoiseStd;};
    void setMNoisePrev(int x){MNoisePrevValue=x;};
    int  getMNoisePrev(){return MNoisePrevValue;};
    
    void setInterval(int x){interval=x;};
    int  getInterval(){return interval;};
    int  getOnsetTime(){return onsetTime.inSamples();};

    void turnNoteOn(MidiBuffer&, int, Counter, bool);
    void turnNoteOff(MidiBuffer&, int, Counter, bool);
    
    void updateParameters(int ID, int channel, int freq, int noteLen, int interval, int velocity);
    
    // counter functions...
    void iterate(MidiBuffer&, int, Counter&, std::vector <bool>&);
    void kill(MidiBuffer&);
    void reset();
    Counter numberOfNoteOns;
    Counter numberOfNoteOffs;
    
private:
    void resetOffsetCounter() {countdownToOffset.reset();};
    bool requiresNoteOn(Counter);
    bool requiresNoteOff();
    void printTapTime(Counter, String);
    
    int noteLen=0, MIDIChannel=1, tapperID=1,
    tapperFreq=1, tapperVel=1, /*should both be assignable to MIDI*/
    interval=22050, beatDivision=2;            /*overwrite from host*/
    
    Counter countdownToOffset, onsetTime;

    bool noteActive = false;
    // LPC params...
    int TKNoiseStd, MNoiseStd, MNoisePrevValue;
};



//==============================================================================
//========= Generator ==========================================================
//==============================================================================
class TapGenerator
{
public:
    TapGenerator(int, double, int, String);
    ~TapGenerator();
    
    void updateBPM(double x); // used to get playhead Info into the tap generator
    void setFrameLen(int x){frameLen=x;};
    void setFs(int x){fs = x;};
    
    void nextBlock(MidiBuffer& midiMessages, Counter& globalCounter, int blockSize);
    void killActiveTappers(MidiBuffer& midiMessages);
    bool allNotesHaveBeenTriggered();
    
    void updateInputTapper(MidiBuffer& midiMessages, Counter globalCounter);
    void resetTriggeredFlags();
    void updateTapAcceptanceWindow();
    void reset();
    
    void readPitchListFromMidiSeq(const OwnedArray<MidiMessageSequence>& midiValuesFromFile);
    void printPitchList();
    void updateTappersPitch(int noteNum);
    void setLocalDataPath(String x){localDataPath = x;};
    
    // public tappers so they can be easily updated by the processor/editor...
    OwnedArray<Tapper>  synthesizedTappers;
    Tapper inputTapper;
    
private:
    
    // private fns...
    void transformNoise();
    void transformLPC();
    void logResults(String);
    
    // tappers...
    int numSynthesizedTappers;
    
    std::vector <bool> notesTriggered; // change these to ownedArrays or scopedPointers??
    std::vector <int> prevAsynch;
    
    // timer params...
    int     TKInterval   = 22050, /*overwrite these values from host*/
            frameLen     = 1024;
    double  bpm          = 120.f,
            fs           = 44100.f,
            beatDivision = 2.f;
    OwnedArray<Array<double>> alpha, asynch;

    Random rand;
    Counter beatCounter, numberOfInputTaps;
    
    // for calculating the moving window of acceptance...
    std::vector <int> prevTapTimes;
    int inputTapAcceptanceWindow, nextWindowThreshold=TKInterval*1.5; //SET THIS PROPERLY!!!
    bool userInputDetected=false;
    
    ScopedPointer<FileOutputStream> captainsLog; // for logging the results
    
    // list of Freqs to feed Tappers...
    OwnedArray<Array<double>> pitchList;
    
    // local data path...
    String localDataPath; // to be fed into the constructor by the processor
    
    int trialNum = 1;
};




//    // LPC model params...
//    float   **alpha, **A,           /*gains and asynchronies*/
//            *Hn, *Mn, *Tn, *Mprev,  /*noise*/
//            *sigmaM, *sigmaT,       /*noise params*/
//            *t;                     /*onset times*/
#endif /* TapManager_hpp */

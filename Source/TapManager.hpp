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

// tapping workflow...
// 0 - procesor has a global counter that gets passed into the tapGenerator
// 1 - tapGenerator uses the counter to call the tappers
// 2 - tapper manages the noteOn and noteOff messages
//
// - Next steps: can the number of tappers be selected at run time by deleting and recreating the tap-manager?
// - Also, can MIDI be fed in rather than static freqs?


// ------- Todo
// how will perturbations be added using the LPC model?
    // how does the window of accpetance work in this case??
// how do the tapper params get updated? do UI params need to passed into the tap generator?
// how can we correctly inherit the blocksize from the host?

// ----------
// why do 9 tappers get allocated when only 1 is used?
// make sure the tapping params all get updated and calculated in the right place! 


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
    void set(long int x){counter = x;};
    //return the counter in different formats...
    long int inSamples(){return counter;};
    double   inFrames(int frameLen){return (double)counter/(double)frameLen;};
    double   inSeconds(double fs){return (double)counter/fs;};
    double   inMilliseconds(double fs){return inSeconds(fs)*1000.0;};
    
private:
    long int counter=0;
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
    void setTKNoise(int x){TKNoiseMS=x;};
    int  getTKNoise(){return TKNoiseMS;};
    void setMNoise(int x){MNoise=x;};
    int  getMNoise(){return MNoise;};
    void setInterval(int x){interval=x;};
    int  getInterval(){return interval;};
    int  getOnsetTime(){return onsetTime.inSamples();};

    void turnNoteOn(MidiBuffer&, int, Counter, bool);
    void turnNoteOff(MidiBuffer&, int, Counter, bool);
    
    void updateParameters(int ID, int channel, int freq, int noteLen, int interval, int velocity);
    
    // counter functions...
    void iterate(MidiBuffer&, int, Counter&, std::vector <bool>&);
    void kill(MidiBuffer&);
    
private:

    void resetOffsetCounter() {offseCounter.reset();};
    bool requiresNoteOn(Counter);
    bool requiresNoteOff();
    void printTapTime(Counter, String);

    
    int noteLen=0, MIDIChannel=1, tapperID=1,
    tapperFreq=1, tapperVel=1, /*should both be assignable to MIDI*/
    interval=22050, beatDivision=2;            /*overwrite from host*/
    
    Counter offseCounter, onsetTime;
    int noteNumber=0;

    bool noteActive = false;
    // LPC params...
    int TKNoiseMS, MNoise;
};



//==============================================================================
//========= Generator ==========================================================
//==============================================================================
class TapGenerator
{
public:
    TapGenerator(int, double, int);
    ~TapGenerator();
    void updateBPM(double x); // used to get playhead Info into the tap generator
    
    void setFrameLen(int x){frameLen=x;};
    void setFs(int x){fs = x;};
    
    void nextBlock(MidiBuffer&, Counter&);
    void killActiveTappers(MidiBuffer&);
    void transform();
    bool allNotesHaveBeenTriggered();
    
    void updateInputTapper(MidiBuffer&, Counter);
    void resetTriggeredFlags();
    void updateTapAcceptanceWindow();
    
private:
    // tappers...
    int numSynthesizedTappers;
    OwnedArray<Tapper>  synthesizedTappers;
    
    Tapper inputTapper;
    
    std::vector <bool> notesTriggered; // change these to ownedArrays or scopedPointers??
    std::vector <int> prevAsynch;
    
    // timer params...
    int TKInterval = 22050; /*overwrite from host*/
    double  bpm = 120.f,    /*overwrite from host*/
    fs  = 44100.f,          /*overwrite from host*/
    beatDivision = 2.f;     /*overwrite using input param*/
    int     frameLen;       /*overwrite from host*/

    Random rand;
    Counter beatCounter, numberOfInputTaps;
    
    // for calculating the moving window of acceptance...
    std::vector <int> prevTapTimes;
    int inputTapAcceptanceWindow, nextWindowThreshold=44100+10000; //SET THIS PROPERLY!!!
    bool userInputDetected=false;
    bool inputWindowExists();
};





//    // LPC model params...
//    float   **alpha, **A,           /*gains and asynchronies*/
//            *Hn, *Mn, *Tn, *Mprev,  /*noise*/
//            *sigmaM, *sigmaT,       /*noise params*/
//            *t;                     /*onset times*/
#endif /* TapManager_hpp */

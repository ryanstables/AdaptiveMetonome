//
//  TapGenerator.hpp
//  adaptiveTappers
//
//  Created by Ryan Stables on 18/02/2017.
//
#ifndef TapGenerator_h
#define TapGenerator_h
#include "../JuceLibraryCode/JuceHeader.h"
#endif /* TapGenerator_hpp */


// tapping workflow...
// 0 - procesor has a global counter that gets passed into the tapGenerator
// 1 - tapGenerator uses the counter to call the tappers
// 2 - tapper manages the noteOn and noteOff messages
//
// - MIDIBuffer is passed by reference into a generator.next() function?
// - Next steps: can the number of tappers be selected at run time by deleting and recreating the tap-manager?
// - Also, can MIDI be fed in rather than static freqs?

// problems...
// how to deal with human input tapper?
// how to do the phase correction?
// how to input midi notes?


// individual tapper object...
class Tapper
{
public:
    Tapper();
    ~Tapper();
    void setNoteLen(int x){noteLen=x;};
    int  getNoteLen(){return noteLen;};
    void setVel(int x){tapperVel=x;};
    int  getVel(){return tapperVel;};
    void setFreq(int x){tapperFreq=x;};
    int  getFreq(){return tapperFreq;};
    bool isActive(){return noteActive;};
    void setTKNoise(int x){TKNoiseMS=x;};
    int  getTKNoise(){return TKNoiseMS;};
    void setMNoise(int x){MNoise=x;};
    int  getMNoise(){return MNoise;};
    
    // send messages to the midi buffer...
    void noteOn(MidiBuffer&, int);
    void noteOff(MidiBuffer&, int);

private:
    int  noteLen, MIDIChannel, tapperID,
         tapperFreq, tapperVel; /*should both be assignable to MIDI*/
    bool noteActive = false;
    // LPC params...
    int TKNoiseMS, MNoise;
};




// tap-manager object...
class TapGenerator
{
public:
    TapGenerator(int);
    ~TapGenerator();
    void setBMP(double x){bpm = x; TKInterval = static_cast <int> (floor(60.f/bpm*fs));};
    double getBMP(){return bpm;};
    //update the midi buffer with the output of the tappers
    // is there another way to update the buffer rathr than passing it very time?
    void next(MidiBuffer&, int);
    
private:
    // counter params...
    int globalCounter, nTappers;
    Tapper *tappers;
    
    // LPC model params...
    float   **alpha, **A,           /*gains and asynchronies*/
            *Hn, *Mn, *Tn, *Mprev,  /*noise*/
            *sigmaM, *sigmaT,       /*noise params*/
            *t;                     /*onset times*/
    int TKInterval;                   /*time keeper interval*/

    // timer params...
    double  bpm = 120.0,            /*from host*/
            fs  = 44100.0,          /*from host*/
            beatDivision = 0.f;     /*input param*/
    int     frameLen;               /*from host*/
};


/*
void AdaptiveTappersAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // update host params in tapper object
    tapper.setFrameLen(buffer.getNumSamples()); // do I need to do this?
    tapper.setBPM(playhead.bpm);

 
    // ------- generate synthesized tappers ----------------
    getPlayHead()->getCurrentPosition(playhead);
    tappers.setBPM()
    if (playhead.isPlaying)
        tappers.next(midiMessages);                                                  // generate noteOn and noteOff messages for synthesized tapprers
    else
        tappers.killActiveTappers(midiMessages);                                                // kill the taps if the playhead stops before noteOffs
 
 
 
 }
 */




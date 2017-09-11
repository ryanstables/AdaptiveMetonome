//
//  Tapper.hpp
//  Metro
//
//  Created by Ryan Stables on 07/09/2017.
//
//

#ifndef Tapper_hpp
#define Tapper_hpp

#include <stdio.h>
#include "../JuceLibraryCode/JuceHeader.h"

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
    
    // LPC params...
    Counter onsetTime;
    double TKNoiseStd,
           MNoiseStd,
           MNoisePrevValue;
    
private:
    void resetOffsetCounter() {countdownToOffset.reset();};
    bool requiresNoteOn(Counter);
    bool requiresNoteOff();
    void printTapTime(Counter, String);
    
    int noteLen=0, MIDIChannel=1, tapperID=1,
    tapperFreq=1, tapperVel=1, /*should both be assignable to MIDI*/
    interval=22050, beatDivision=2;            /*overwrite from host*/
    
    Counter countdownToOffset;
    
    bool noteActive = false;
};



#endif /* Tapper_hpp */

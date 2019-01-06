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
#include "Counter.hpp"
#include "../JuceLibraryCode/JuceHeader.h"


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
    double  getMNoiseStdInSamples(double fs){return MNoiseStd/1000.f * fs;};
    double  getTKNoiseStdInSamples(double fs){return TKNoiseStd/1000.f * fs;};
    
    void setInterval(int x){prevInterval = interval; interval=x;};
    int  getInterval(){return interval;};
    int  getPrevInterval(){return prevInterval;};
    int  getOnsetTime(){return onsetTime.inSamples();};
    int  getPrevOnsetTime(){return prevOnsetTime.inSamples();};
    void setNextOnsetTime(int x){nextOnsetTime = x;};
    
    void turnNoteOn(MidiBuffer&, int, Counter, int, bool);
    void turnNoteOff(MidiBuffer&, int, Counter, bool);
    
    void updateParameters(int ID, int channel, int freq, int noteLen, int interval, int velocity);
    
    // counter functions...
    void iterate(MidiBuffer&, int, Counter, int, std::vector <bool>&);
    void kill(MidiBuffer&);
    void reset();
    Counter numberOfNoteOns;
    Counter numberOfNoteOffs;
    
    // LPC params...
    Counter onsetTime, prevOnsetTime;
    double TKNoiseStd,
    MNoiseStd,
    MNoisePrevValue;
    
private:
    void resetOffsetCounter() {countdownToOffset.reset();};
    bool requiresNoteOn(Counter);
    bool requiresNoteOff();
    void printTapTime(Counter, String);
    
    int noteLen, MIDIChannel, tapperID=1,
    tapperFreq=1, tapperVel=1, /*should both be assignable to MIDI*/
    interval=22050, prevInterval = 22050, beatDivision=2,            /*overwrite from host*/
    globalBeatNumber = 0,   // inject the beat number from the TapManager to check for valid prevOnsetTimes
    nextOnsetTime;
    
    Counter countdownToOffset;
    
    bool noteActive = false;
};




#endif /* Tapper_hpp */

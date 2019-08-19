//
//  TapManager.hpp
//  Metro
//
//  Created by Ryan Stables on 24/02/2017.
//

#ifndef TapManager_hpp
#define TapManager_hpp

#include "../JuceLibraryCode/JuceHeader.h"
#include "Counter.hpp"
#include "Tapper.hpp"
#include "PreloadedMIDI.h"
#include <vector>

// -------
// tapping workflow...
// 0 - procesor has a global counter that gets passed into the tapGenerator
// 1 - tapGenerator uses the counter to call the tappers
// 2 - tapper manages the noteOn and noteOff messages
// Notes:
// if there are more than 1 taps at a detected beat, only the first is chosen.
// the LPC model only gets applied when there is a user input.

// ==============================================================================
// ========= Generator ==========================================================
// ==============================================================================
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
    
    void updateInputTapper(MidiBuffer& midiMessages, Counter globalCounter, int beatNumber);
    void resetTriggeredFlags();
    void updateTapAcceptanceWindow();
    void reset();
    
    void readPitchListFromMidiSeq(const OwnedArray<MidiMessageSequence>& midiValuesFromFile);
    void printPitchList();
    void updateTappersPitch(int noteNum);
    void setLocalDataPath(String x){localDataPath = x;};
    
    void setAlpha(int row, int col, double value){alpha[row]->set(col, value);};
    double getAlpha(int row, int col){return alpha[row]->getUnchecked(col);};
    
    // public tappers so they can be easily updated by the processor/editor...
    
    int numSynthesizedTappers;
    OwnedArray<Tapper>  synthesizedTappers;
    Tapper inputTapper;
    OwnedArray<Array<double>> alpha; //alpha can be updated by the UI
    Array <int> velocities; // populated by the processor
    
    void initLogFile();
    
private:
    
    // private fns...
    void transformNoise(int randWindowMs);
    void transformLPC();
    void logResults(String);
    double getRandomValue(double std);
    double meanSynthesizedOnsetTime();
    void readPitchListFromPreloadedArray();
    
    std::vector <bool> notesTriggered; // change these to ownedArrays or scopedPointers??
    std::vector <int> prevAsynch;
    
    // timer params...
    int     frameLen     = 1024;
    double  bpm          = 120.f,
            fs           = 44100.f,
            beatDivision = 2.f;
    int numIntroBeeps = 4;
    Random rand;
    Counter beatCounter, numberOfInputTaps, trialNum;
    
    // for calculating the moving window of acceptance...
    bool userInputDetected=false;
    ScopedPointer<FileOutputStream> captainsLog; // for logging the results
    // list of Freqs to feed Tappers...
    OwnedArray<Array<double>> pitchList;
    PreloadedMIDI preloadedHaydn;
    // local data path...
    String localDataPath; // to be fed into the constructor by the processor
    // LPC Parameters...
    OwnedArray<Array<double>> asynch, asynchAlpha;
    int     TKInterval   = 22050; /*overwrite these values from host*/
    int inputTapAcceptanceWindow, nextWindowThreshold=TKInterval*1.5; //SET THIS PROPERLY!!!
    
    // Strings for logfile
    String TKNoiseStr, MNoiseStr, alphaStr, asyncStr, TKNParamStr, MNParamStr, volStr, baseString;
    int currentBeatWithNoteOn;
};

#endif /* TapManager_hpp */

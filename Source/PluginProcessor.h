/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED
#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/
class AdaptiveTappersAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    AdaptiveTappersAudioProcessor();
    ~AdaptiveTappersAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    
    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // interface parameters...
    AudioParameterFloat *FreqParam[3], *DurParam[3], *VelParam[3];
    AudioParameterBool *recordEnabled;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdaptiveTappersAudioProcessor)
    
    // tapping functions-------------
    void generateTappers(MidiBuffer&);
    void killActiveTappers(MidiBuffer&);
    int transformIOI(int, double);
    
    bool allTapsAreNew();
    void getMeanTaps();
    
    // tapping variables-------------
    Synthesiser synth;
    AudioPlayHead::CurrentPositionInfo playhead;
    ScopedPointer <FileOutputStream> captainsLog;
    
    long int frameCounter   = 0,
             noteCounter    = 0,
             beatCounter    = 0;
    
    Random  rand;
    
    int     interval                        = 0,
            beatPos                         = 0,
            numSynthesizedTappers           = 3,
            randWindowMs                    = 100,
            synthesizedTapperInterval[3]    = {0, 0, 0},
            synthesizedTapperNoteLen[3]     = {0, 0, 0},
            noteOnCounter[3]                = {0, 0, 0},
            noteOffCounter[3]               = {0, 0, 0};
 
    bool    noteCurrentlyActive[3] = {false, false, false},
            channelHasNewValue[3]  = {false, false, false};
    
    double  bpm,                    /*from host*/
            fs,                     /*from host*/
            frameLen,               /*from host*/
            beatDivision            = 0.f,
            prevTapTimes[4]         = {0.0, 0.0, 0.0, 0.0},
            currentTapTimes[4]      = {0.0, 0.0, 0.0, 0.0},
            currentNoteOffTimes[4]  = {0.0, 0.0, 0.0, 0.0},
            meanTapTime             = 0.f;
};


#endif  // PLUGINPROCESSOR_H_INCLUDED

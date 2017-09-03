/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class MetroAudioProcessorEditor  :  public AudioProcessorEditor,
                                    public Slider::Listener,
                                    public Timer
{
public:
    MetroAudioProcessorEditor (MetroAudioProcessor&);
    ~MetroAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged(Slider *s) override;
    void timerCallback() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MetroAudioProcessor& processor;
    int width = 500, height = 300;

    int sliderwidth = 50;

    void addVelSlider(Slider &s, int xOffset, int yOffset);
    
    // sliders to control the velocities of the tappers...
    // change to an owned array
    Slider slider1, velTapper1, velTapper2, velTapper3;

    int numSynthTappers;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED

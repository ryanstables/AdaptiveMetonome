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
                                    public Button::Listener,
                                    public Timer
{
public:
    MetroAudioProcessorEditor (MetroAudioProcessor&);
    ~MetroAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged(Slider *s) override;
    void buttonClicked(Button *b) override;
    void timerCallback() override;
    
private:
    
    void openMidiFile();
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MetroAudioProcessor& processor;
    int width, height, xOffset, yOffset, sliderwidth, sliderheight;
    
    void addButton(Button &b, int xOffset, int yOffset);
    void addVelSlider(Slider &s, int xOffset, int yOffset);
    void addRotarySlider(Slider &s, int xOffset, int yOffset);


    // sliders to control the velocities of the tappers...
    Slider slider1, velTapper1, velTapper2, velTapper3;
    // sliders to control the noise of the synth tappers...
    Slider TKNoiseSlider1, TKNoiseSlider2, TKNoiseSlider3;

    TextButton openButton;
    int numSynthTappers;
    
    Label filePathLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED

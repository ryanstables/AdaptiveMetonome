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
class AdaptiveTappersAudioProcessorEditor  : public AudioProcessorEditor,
                                             public Slider::Listener,
                                             public TextButton::Listener,
                                             private Timer
{
public:
    AdaptiveTappersAudioProcessorEditor (AdaptiveTappersAudioProcessor&);
    ~AdaptiveTappersAudioProcessorEditor();

    void timerCallback() override;
    void sliderValueChanged (Slider *slider) override;
    void buttonClicked(Button *b) override;
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:

    AdaptiveTappersAudioProcessor& processor;

    Slider  t1Freq, t1Dur, t1Amp,
            t2Freq, t2Dur, t2Amp,
            t3Freq, t3Dur, t3Amp;
    TextButton recordButton;
    int width, height;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdaptiveTappersAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED

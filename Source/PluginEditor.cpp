/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AdaptiveTappersAudioProcessorEditor::AdaptiveTappersAudioProcessorEditor (AdaptiveTappersAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p)
{
    // set editor's size...
    width = 610;
    height = 150;
    setSize (width, height);

    
    // freq of first tapper...
    addAndMakeVisible(t1Freq);
    t1Freq.setRange (0.f, 127.f, 1.f);
    t1Freq.setValue(68.f);
    t1Freq.addListener (this);
    // dur of first tapper..
    addAndMakeVisible(t1Dur);
    t1Dur.setRange (2.f, 16.f, 1.f);
    t1Dur.setValue(4.f);
    t1Dur.addListener (this);
    // vel of first tapper...
    addAndMakeVisible(t1Amp);
    t1Amp.setRange (0.f, 127.f, 1.f);
    t1Amp.setValue(90);
    t1Amp.addListener (this);
    
    // freq of 2nd tapper...
    addAndMakeVisible(t2Freq);
    t2Freq.setRange (0.f, 127.f, 1.f);
    t2Freq.setValue(80.f);
    t2Freq.addListener (this);
    // dur of 2nd tapper..
    addAndMakeVisible(t2Dur);
    t2Dur.setRange (2.f, 16.f, 1.f);
    t2Dur.setValue(4.f);
    t2Dur.addListener (this);
    // vel of 2ns tapper...
    addAndMakeVisible(t2Amp);
    t2Amp.setRange (0.f, 127.f, 1.f);
    t2Amp.setValue(60.f);
    t2Amp.addListener (this);

    // freq of 3rd tapper...
    addAndMakeVisible(t3Freq);
    t3Freq.setRange (0.f, 127.f, 1.f);
    t3Freq.setValue(92.f);
    t3Freq.addListener (this);
    // dur of 3rd tapper..
    addAndMakeVisible(t3Dur);
    t3Dur.setRange (2.f, 16.f, 1.f);
    t3Dur.setValue(4.f);
    t3Dur.addListener (this);
    // vel of 3rd tapper...
    addAndMakeVisible(t3Amp);
    t3Amp.setRange (0.f, 127.f, 1.f);
    t3Amp.setValue(60.f);
    t3Amp.addListener (this);
    
    addAndMakeVisible(recordButton);
    recordButton.setColour(TextButton::buttonColourId, Colours::dimgrey);
    recordButton.setColour(TextButton::buttonOnColourId, Colours::red);
    recordButton.setColour(TextButton::textColourOffId, Colours::red);
    recordButton.setColour(TextButton::textColourOnId, Colours::dimgrey);
    recordButton.setButtonText("Record");
    recordButton.setToggleState(false, dontSendNotification);
    recordButton.addListener(this);

    
    t1Freq.setBounds (10+(width/3) *0, 10, (width/3)-20, 20);
    t1Dur.setBounds  (20+(width/3) *1, 10, (width/3)-20, 20);
    t1Amp.setBounds  (30+(width/3) *2, 10, (width/3)-20, 20);
    t2Freq.setBounds (10+(width/3) *0, 40, (width/3)-20, 20);
    t2Dur.setBounds  (20+(width/3) *1, 40, (width/3)-20, 20);
    t2Amp.setBounds  (30+(width/3) *2, 40, (width/3)-20, 20);
    t3Freq.setBounds (10+(width/3) *0, 70, (width/3)-20, 20);
    t3Dur.setBounds  (20+(width/3) *1, 70, (width/3)-20, 20);
    t3Amp.setBounds  (30+(width/3) *2, 70, (width/3)-20, 20);
    recordButton.setBounds(10, height-30, width-20, 20);
    
    // start the listner to update params...
    startTimerHz (25);
}

AdaptiveTappersAudioProcessorEditor::~AdaptiveTappersAudioProcessorEditor()
{
}

//==============================================================================
void AdaptiveTappersAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void AdaptiveTappersAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}


void AdaptiveTappersAudioProcessorEditor::timerCallback()
{
    t1Freq.setValue(*processor.FreqParam[0], dontSendNotification);
    t1Dur.setValue(*processor.DurParam[0], dontSendNotification);
    t1Amp.setValue(*processor.VelParam[0], dontSendNotification);
    
    t2Freq.setValue(*processor.FreqParam[1], dontSendNotification);
    t2Dur.setValue(*processor.DurParam[1], dontSendNotification);
    t2Amp.setValue(*processor.VelParam[1], dontSendNotification);

    t3Freq.setValue(*processor.FreqParam[2], dontSendNotification);
    t3Dur.setValue(*processor.DurParam[2], dontSendNotification);
    t3Amp.setValue(*processor.VelParam[2], dontSendNotification);

    recordButton.setToggleState(*processor.recordEnabled, dontSendNotification);
}


void AdaptiveTappersAudioProcessorEditor::sliderValueChanged (Slider *s)
{
    // ----------- 1st tapper ------------------
    if(s==&t1Freq)
    {
        *processor.FreqParam[0] = (float)t1Freq.getValue();
    }
    else if (s==&t1Dur)
    {
        *processor.DurParam[0] = (float)t1Dur.getValue();
    }
    else if (s==&t1Amp)
    {
        *processor.VelParam[0] = (float)t1Amp.getValue();
    }
    // ----------- 2nd tapper ------------------
    else if(s==&t2Freq)
    {
        *processor.FreqParam[1] = (float)t2Freq.getValue();
    }
    else if (s==&t2Dur)
    {
        *processor.DurParam[1] = (float)t2Dur.getValue();
    }
    else if (s==&t2Amp)
    {
        *processor.VelParam[1] = (float)t2Amp.getValue();
    }
    // ----------- 3rd tapper ------------------
    else if(s==&t3Freq)
    {
        *processor.FreqParam[2] = (float)t3Freq.getValue();
    }
    else if (s==&t3Dur)
    {
        *processor.DurParam[2] = (float)t3Dur.getValue();
    }
    else if (s==&t3Amp)
    {
        *processor.VelParam[2] = (float)t3Amp.getValue();
    }
}

void AdaptiveTappersAudioProcessorEditor::buttonClicked(Button *b)
{
    if (b==&recordButton)
    {
        if(recordButton.getToggleState())
        {
            recordButton.setButtonText("Record");
            recordButton.setToggleState(false, dontSendNotification);
        }
        else
        {
            recordButton.setButtonText("Recording...");
            recordButton.setToggleState(true, dontSendNotification);
        }
        
        *processor.recordEnabled = recordButton.getToggleState();
    }
}

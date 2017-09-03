/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
MetroAudioProcessorEditor::MetroAudioProcessorEditor (MetroAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{

    // input tapper vel...
    addVelSlider(slider1, xOffset, yOffset);
    // synth Tapper Vels...
    addVelSlider(velTapper1, 10+xOffset+1*sliderwidth, yOffset);
    addVelSlider(velTapper2, 20+xOffset+2*sliderwidth, yOffset);
    addVelSlider(velTapper3, 30+xOffset+3*sliderwidth, yOffset);
    
    // set the window size...
    setSize (width, height);
    
    //start the timer (every 25ms)...
    startTimer(25);
}

MetroAudioProcessorEditor::~MetroAudioProcessorEditor()
{
}

void MetroAudioProcessorEditor::addVelSlider(Slider &s, int xOffset, int yOffset)
{
    // init the velocity sliders...
    addAndMakeVisible(s);
    s.setRange(0.f, 127.f, 1.f);
    s.setValue(127);
    s.setBounds(xOffset, yOffset, sliderwidth, height-80);
    s.setSliderStyle(Slider::LinearVertical);
    
    s.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderwidth, 20);
    s.setTextBoxIsEditable(true);
    s.addListener(this);
}


void MetroAudioProcessorEditor::sliderValueChanged(Slider *s)
{
    // update stuff based on the slider...
    if(s == &slider1)
    {
        processor.updateInputTapperVelocity(slider1.getValue());
    }
    else if(s == &velTapper1)
    {
        processor.updatedSynthTapperVelocity(0, velTapper1.getValue());
    }
    else if(s == &velTapper2)
    {
        processor.updatedSynthTapperVelocity(1, velTapper2.getValue());
    }
    else if(s == &velTapper3)
    {
        processor.updatedSynthTapperVelocity(2, velTapper3.getValue());
    }
}


void MetroAudioProcessorEditor::timerCallback()
{
    // input tapper vel...
    slider1.setValue(*processor.gainsParam, dontSendNotification);
    velTapper1.setValue(*processor.velParam1, dontSendNotification);
    velTapper2.setValue(*processor.velParam2, dontSendNotification);
    velTapper3.setValue(*processor.velParam3, dontSendNotification);
}


//==============================================================================
void MetroAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
    String filePath = processor.LocalDataPath+processor.midiFileName;
    g.drawFittedText("MIDI: "+filePath, 10, 10, width-20, 30, Justification::left, 3);

//    g.drawFittedText("v2", 10+xOffset+1*sliderwidth,  height-10, width-20, 30, Justification::left, 3);
//    g.drawFittedText("vio", 20+xOffset+1*sliderwidth, height-10, width-20, 30, Justification::left, 3);
//    g.drawFittedText("cel", 30+xOffset+1*sliderwidth, height-10, width-20, 30, Justification::left, 3);
//    g.drawFittedText("cel", 30+xOffset+1*sliderwidth, height-10, width-20, 30, Justification::left, 3);
}

void MetroAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

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
    int sliderwidth = 50;
    
    // init the velocity sliders...
    addAndMakeVisible(slider1);
    slider1.setRange(0.f, 127.f, 1.f);
    slider1.setValue(127);
    slider1.setBounds(10, 50, sliderwidth, height-80);
    slider1.setSliderStyle(Slider::LinearVertical);
    
    slider1.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderwidth, 20);
    slider1.setTextBoxIsEditable(true);
    
    
    // set the window size...
    setSize (width, height);
    
    //start the timer (every 25ms)...
    startTimer(25);
}

MetroAudioProcessorEditor::~MetroAudioProcessorEditor()
{
}


void MetroAudioProcessorEditor::sliderValueChanged(Slider *s)
{
    // update stuff based on the slider...
    if(s == &slider1)
    {
        *processor.gainsParam = slider1.getValue();
    }
}

void MetroAudioProcessorEditor::timerCallback()
{
    slider1.setValue(*processor.gainsParam, dontSendNotification);
}

//==============================================================================
void MetroAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
    String filePath = processor.LocalDataPath+processor.midiFileName;
    g.drawFittedText("MIDI: "+filePath, 10, 10, width-20, 30, Justification::left, 3);
    
    
}

void MetroAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

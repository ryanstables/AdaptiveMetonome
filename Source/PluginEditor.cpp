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

    String filePath = processor.LocalDataPath+processor.midiFileName;

    // file path label...
    addAndMakeVisible(filePathLabel);
    filePathLabel.setBounds(10, 10, width-20, 30);
    filePathLabel.setText("MIDI: "+filePath, dontSendNotification);
    
    
    // input tapper vel...
    addVelSlider(slider1, xOffset, yOffset);
    
    // synth Tapper Vels...
    addVelSlider(velTapper1, 10+xOffset+1*sliderwidth, yOffset);
    addVelSlider(velTapper2, 20+xOffset+2*sliderwidth, yOffset);
    addVelSlider(velTapper3, 30+xOffset+3*sliderwidth, yOffset);
    
    //add the openFile button...
    addButton(openButton, width-70, 50);
    
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


void MetroAudioProcessorEditor::addButton(Button &b, int xOffset, int yOffset)
{
    addAndMakeVisible(b);
    b.setButtonText("open...");
    b.setColour (TextButton::buttonColourId, Colours::green);
    b.setBounds(xOffset, yOffset, 50, 20);
    b.addListener(this);
}



void MetroAudioProcessorEditor::openMidiFile()
{
    FileChooser chooser("Select a MIDI file...", File::nonexistent);
    if(chooser.browseForFileToOpen())
    {
        // load file chooser and update processor...
        File midiFile(chooser.getResult());
        processor.updateMIDIFile(midiFile.getFullPathName());
        
        //update the label on the UI...
        filePathLabel.setText("MIDI: "+midiFile.getFullPathName(), dontSendNotification);
    }
}


void MetroAudioProcessorEditor::buttonClicked(Button *b)
{
    if(b == &openButton)
    {
     openMidiFile();
    }
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
    
    // filename...
//    String filePath = processor.LocalDataPath+processor.midiFileName;
//    g.drawFittedText("MIDI: "+filePath, 10, 10, width-20, 30, Justification::left, 3);
    
    // instructions...
    g.setFont(24.f);
    g.setColour(Colours::red);
    g.drawMultiLineText("Wait for 4 beeps, then start tapping along...", 100+xOffset+4*sliderwidth, 100, 150);

    // labels for vel faders...
    g.setFont(15.f);
    g.setColour(Colours::blue);
    g.drawFittedText(	"v1",  xOffset, height-20, sliderwidth, 10, Justification::centred, 1, 0.0f);
    g.drawFittedText(	"v2",  10+xOffset+1*sliderwidth, height-20, sliderwidth, 10, Justification::centred, 1, 0.0f);
    g.drawFittedText(	"vio", 20+xOffset+2*sliderwidth, height-20, sliderwidth, 10, Justification::centred, 1, 0.0f);
    g.drawFittedText(	"cel", 30+xOffset+3*sliderwidth, height-20, sliderwidth, 10, Justification::centred, 1, 0.0f);
}

void MetroAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

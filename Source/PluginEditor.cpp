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
    // UI params...
    width = 600;
    height = 400;
    xOffset = 10;
    yOffset = 50;
    sliderwidth = 50;
    sliderheight = 120;
    
    // set the window size...
    setSize (width, height);
    
    String filePath = processor.LocalDataPath+processor.midiFileName;

    // file path label...
    addAndMakeVisible(filePathLabel);
    filePathLabel.setBounds(xOffset, 20, width-20, 30);
    filePathLabel.setText("MIDI: "+filePath, dontSendNotification);
    
    //add the openFile button...
    addButton(openButton, width-70, 20);

    
    // input tapper vel...
    addVelSlider(slider1, xOffset, yOffset+20);
    
    // synth Tapper sliders...
    addVelSlider(velTapper1, 10+xOffset+1*sliderwidth, yOffset+20);
    addRotarySlider(TKNoiseSlider1, 10+xOffset+1*sliderwidth, yOffset+sliderheight+40);
    addRotarySlider(MNoiseSlider1,  10+xOffset+1*sliderwidth, yOffset+sliderheight+sliderwidth+50);
    
    addVelSlider(velTapper2, 20+xOffset+2*sliderwidth, yOffset+20);
    addRotarySlider(TKNoiseSlider2, 20+xOffset+2*sliderwidth, yOffset+sliderheight+40);
    addRotarySlider(MNoiseSlider2,  20+xOffset+2*sliderwidth, yOffset+sliderheight+sliderwidth+50);
    
    addVelSlider(velTapper3, 30+xOffset+3*sliderwidth, yOffset+20);
    addRotarySlider(TKNoiseSlider3, 30+xOffset+3*sliderwidth, yOffset+sliderheight+40);
    addRotarySlider(MNoiseSlider3,  30+xOffset+3*sliderwidth, yOffset+sliderheight+sliderwidth+50);
    
    // alphas...
    int halfwidth = width/2;
    addRotarySlider(p1p1, halfwidth+1*sliderwidth,    yOffset+20);
    addRotarySlider(p1p2, 10+halfwidth+2*sliderwidth, yOffset+20);
    addRotarySlider(p1p3, 20+halfwidth+3*sliderwidth, yOffset+20);
    addRotarySlider(p1p4, 30+halfwidth+4*sliderwidth, yOffset+20);

    addRotarySlider(p2p1, halfwidth+1*sliderwidth,    yOffset+30+sliderwidth);
    addRotarySlider(p2p2, 10+halfwidth+2*sliderwidth, yOffset+30+sliderwidth);
    addRotarySlider(p2p3, 20+halfwidth+3*sliderwidth, yOffset+30+sliderwidth);
    addRotarySlider(p2p4, 30+halfwidth+4*sliderwidth, yOffset+30+sliderwidth);
    
    addRotarySlider(p3p1, halfwidth+1*sliderwidth,    yOffset+40+2*sliderwidth);
    addRotarySlider(p3p2, 10+halfwidth+2*sliderwidth, yOffset+40+2*sliderwidth);
    addRotarySlider(p3p3, 20+halfwidth+3*sliderwidth, yOffset+40+2*sliderwidth);
    addRotarySlider(p3p4, 30+halfwidth+4*sliderwidth, yOffset+40+2*sliderwidth);

    addRotarySlider(p4p1, halfwidth+1*sliderwidth,    yOffset+50+3*sliderwidth);
    addRotarySlider(p4p2, 10+halfwidth+2*sliderwidth, yOffset+50+3*sliderwidth);
    addRotarySlider(p4p3, 20+halfwidth+3*sliderwidth, yOffset+50+3*sliderwidth);
    addRotarySlider(p4p4, 30+halfwidth+4*sliderwidth, yOffset+50+3*sliderwidth);

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
    s.setBounds(xOffset, yOffset, sliderwidth, sliderheight);
    s.setSliderStyle(Slider::LinearVertical);
    // text box...
    s.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderwidth, 20);
    s.setTextBoxIsEditable(true);
    s.addListener(this);
}

void MetroAudioProcessorEditor::addRotarySlider(Slider &s, int xOffset, int yOffset)
{
    // init the rotary TK/M noise sliders...
    addAndMakeVisible(s);
    s.setRange(0.0, 50.0, 0.5);
    s.setValue(0.0);
    s.setBounds(xOffset, yOffset, sliderwidth, sliderwidth);
    s.setSliderStyle(Slider::Rotary);
    // text box...
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
    // update the processor using the velocity sliders...
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
    /*
    update the procesor using the rotary noise knobs...
    */
    else if(s == &TKNoiseSlider1)
    {
        processor.updateSynthTapperTKNoise(0, TKNoiseSlider1.getValue());
    }
    else if(s == &TKNoiseSlider2)
    {
        processor.updateSynthTapperTKNoise(1, TKNoiseSlider2.getValue());
    }
    else if(s == &TKNoiseSlider3)
    {
        processor.updateSynthTapperTKNoise(2, TKNoiseSlider2.getValue());
    }
}


void MetroAudioProcessorEditor::timerCallback()
{
    // input tapper vel...
    slider1.setValue(*processor.gainsParam, dontSendNotification);
    velTapper1.setValue(*processor.velParam1, dontSendNotification);
    velTapper2.setValue(*processor.velParam2, dontSendNotification);
    velTapper3.setValue(*processor.velParam3, dontSendNotification);
    
    // noise params from processor...
    TKNoiseSlider1.setValue(*processor.TKNoiseParam1, dontSendNotification);
    TKNoiseSlider2.setValue(*processor.TKNoiseParam2, dontSendNotification);
    TKNoiseSlider3.setValue(*processor.TKNoiseParam3, dontSendNotification);
}


//==============================================================================
void MetroAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);
    
    // filename...
//    String filePath = processor.LocalDataPath+processor.midiFileName;
//    g.drawFittedText("MIDI: "+filePath, 10, 10, width-20, 30, Justification::left, 3);
    
    // instructions...
    g.setFont(25.f);
    g.setColour(Colours::red);
    g.drawMultiLineText("Wait for 4 beeps, then start tapping along...", xOffset, 20, width);

    g.setColour(Colours::black);
    g.fillRect(width/2+1, yOffset, width, height-yOffset);
    
    // labels for vel faders...
    g.setFont(15.f);
    g.setColour(Colours::blue);
    g.drawFittedText(	"v1",  xOffset, yOffset, sliderwidth, 10, Justification::centred, 1, 0.0f);
    g.drawFittedText(	"v2",  10+xOffset+1*sliderwidth, yOffset, sliderwidth, 10, Justification::centred, 1, 0.0f);
    g.drawFittedText(	"vio", 20+xOffset+2*sliderwidth, yOffset, sliderwidth, 10, Justification::centred, 1, 0.0f);
    g.drawFittedText(	"cel", 30+xOffset+3*sliderwidth, yOffset, sliderwidth, 10, Justification::centred, 1, 0.0f);

    // noise label...
    g.setFont(11.f);
    g.setColour(Colours::black);
    g.drawFittedText(    "TkNoise:", xOffset, 50+yOffset+sliderheight, sliderwidth, 10, Justification::left, 1, 0.0f);
    g.drawFittedText(    "MNoise:",  xOffset, 70+yOffset+sliderheight+sliderwidth, sliderwidth, 10, Justification::left, 1, 0.0f);
}


void MetroAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

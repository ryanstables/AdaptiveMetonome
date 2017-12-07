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
    height = 360;
    xOffset = 10;
    yOffset = 50;
    sliderwidth = 50;
    sliderheight = 120;
    
    // set the window size...
    setSize (width, height);
    
    
    //add the openFile button and label...
    MidiFilePath = processor.LocalDataPath+processor.midiFileName;
    addAndMakeVisible(filePathLabel);
    addButton(openButton);
    
    // input tapper vel...
    addVelSlider(slider1);
    
    // synth Tapper sliders...
    addVelSlider(velTapper1);
    addRotarySlider(TKNoiseSlider1);
    addRotarySlider(MNoiseSlider1);
    
    addVelSlider(velTapper2);
    addRotarySlider(TKNoiseSlider2);
    addRotarySlider(MNoiseSlider2);
    
    addVelSlider(velTapper3);
    addRotarySlider(TKNoiseSlider3);
    addRotarySlider(MNoiseSlider3);
    
    // alpha sub-component...
    addAndMakeVisible(alphaMatrix);
    alphaMatrix.b11.addListener(this);
    alphaMatrix.b12.addListener(this);
    alphaMatrix.b13.addListener(this);
    alphaMatrix.b14.addListener(this);
    // -----
    alphaMatrix.b21.addListener(this);
    alphaMatrix.b22.addListener(this);
    alphaMatrix.b23.addListener(this);
    alphaMatrix.b24.addListener(this);
    // -----
    alphaMatrix.b31.addListener(this);
    alphaMatrix.b32.addListener(this);
    alphaMatrix.b33.addListener(this);
    alphaMatrix.b34.addListener(this);
    // -----
    alphaMatrix.b41.addListener(this);
    alphaMatrix.b42.addListener(this);
    alphaMatrix.b43.addListener(this);
    alphaMatrix.b44.addListener(this);

    //start the timer (every 25ms)...
    startTimer(25);
}

MetroAudioProcessorEditor::~MetroAudioProcessorEditor()
{
}

void MetroAudioProcessorEditor::addVelSlider(Slider &s)
{
    // init the velocity sliders...
    addAndMakeVisible(s);
    s.setRange(0.f, 127.f, 1.f);
    s.setValue(127);
    s.setSliderStyle(Slider::LinearVertical);
    // text box...
    s.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderwidth, 20);
    s.setTextBoxIsEditable(true);
    s.addListener(this);
}

void MetroAudioProcessorEditor::addRotarySlider(Slider &s)
{
    // init the rotary TK/M noise sliders...
    addAndMakeVisible(s);
    s.setRange(0.0, 50.0, 0.5);
    s.setValue(0.0);
    s.setSliderStyle(Slider::Rotary);
    // text box...
    s.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, sliderwidth, 20);
    s.setTextBoxIsEditable(true);
    s.addListener(this);
}


void MetroAudioProcessorEditor::addButton(Button &b)
{
    addAndMakeVisible(b);
    b.setButtonText("open...");
    b.setColour (TextButton::buttonColourId, Colours::green);
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
    /*
     Update the alpha params here ------------------
     */
    else if(s == &(alphaMatrix.b11))
    {
        processor.tapManager->setAlpha(0, 0, alphaMatrix.b11.getValue());
    }
    else if(s == &(alphaMatrix.b12))
    {
        processor.tapManager->setAlpha(0, 1, alphaMatrix.b12.getValue());
    }
    else if(s == &(alphaMatrix.b13))
    {
        processor.tapManager->setAlpha(0, 2, alphaMatrix.b13.getValue());
    }
    else if(s == &(alphaMatrix.b14))
    {
        processor.tapManager->setAlpha(0, 3, alphaMatrix.b14.getValue());
    }
    // -------------------
    else if(s == &(alphaMatrix.b21))
    {
        processor.tapManager->setAlpha(1, 0, alphaMatrix.b21.getValue());
    }
    else if(s == &(alphaMatrix.b22))
    {
        processor.tapManager->setAlpha(1, 1, alphaMatrix.b22.getValue());
    }
    else if(s == &(alphaMatrix.b23))
    {
        processor.tapManager->setAlpha(1, 2, alphaMatrix.b23.getValue());
    }
    else if(s == &(alphaMatrix.b24))
    {
        processor.tapManager->setAlpha(1, 3, alphaMatrix.b24.getValue());
    }
    // -------------------
    else if(s == &(alphaMatrix.b31))
    {
        processor.tapManager->setAlpha(2, 0, alphaMatrix.b31.getValue());
    }
    else if(s == &(alphaMatrix.b32))
    {
        processor.tapManager->setAlpha(2, 1, alphaMatrix.b32.getValue());
    }
    else if(s == &(alphaMatrix.b33))
    {
        processor.tapManager->setAlpha(2, 2, alphaMatrix.b33.getValue());
    }
    else if(s == &(alphaMatrix.b34))
    {
        processor.tapManager->setAlpha(2, 3, alphaMatrix.b34.getValue());
    }
    // -------------------
    else if(s == &(alphaMatrix.b41))
    {
        processor.tapManager->setAlpha(3, 0, alphaMatrix.b41.getValue());
    }
    else if(s == &(alphaMatrix.b42))
    {
        processor.tapManager->setAlpha(3, 1, alphaMatrix.b42.getValue());
    }
    else if(s == &(alphaMatrix.b43))
    {
        processor.tapManager->setAlpha(3, 2, alphaMatrix.b43.getValue());
    }
    else if(s == &(alphaMatrix.b44))
    {
        processor.tapManager->setAlpha(3, 3, alphaMatrix.b44.getValue());
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
    
    
    // alpha params from processor...
    alphaMatrix.b11.setValue(processor.tapManager->getAlpha(0, 0), dontSendNotification);
    alphaMatrix.b12.setValue(processor.tapManager->getAlpha(0, 1), dontSendNotification);
    alphaMatrix.b13.setValue(processor.tapManager->getAlpha(0, 2), dontSendNotification);
    alphaMatrix.b14.setValue(processor.tapManager->getAlpha(0, 3), dontSendNotification);
    // ------
    alphaMatrix.b21.setValue(processor.tapManager->getAlpha(1, 0), dontSendNotification);
    alphaMatrix.b22.setValue(processor.tapManager->getAlpha(1, 1), dontSendNotification);
    alphaMatrix.b23.setValue(processor.tapManager->getAlpha(1, 2), dontSendNotification);
    alphaMatrix.b24.setValue(processor.tapManager->getAlpha(1, 3), dontSendNotification);
    // ------
    alphaMatrix.b31.setValue(processor.tapManager->getAlpha(2, 0), dontSendNotification);
    alphaMatrix.b32.setValue(processor.tapManager->getAlpha(2, 1), dontSendNotification);
    alphaMatrix.b33.setValue(processor.tapManager->getAlpha(2, 2), dontSendNotification);
    alphaMatrix.b34.setValue(processor.tapManager->getAlpha(2, 3), dontSendNotification);
    // ------
    alphaMatrix.b41.setValue(processor.tapManager->getAlpha(3, 0), dontSendNotification);
    alphaMatrix.b42.setValue(processor.tapManager->getAlpha(3, 1), dontSendNotification);
    alphaMatrix.b43.setValue(processor.tapManager->getAlpha(3, 2), dontSendNotification);
    alphaMatrix.b44.setValue(processor.tapManager->getAlpha(3, 3), dontSendNotification);
}


//==============================================================================
void MetroAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colour::fromRGB(85, 95, 160));
    g.setFont(25.f);
    g.setColour(Colours::yellow);
    g.drawMultiLineText("Wait for 4 beeps, then start tapping along...", xOffset, 25, getWidth());
    
    int topTextBarHeight = 50,
    bottomBarHeight = 50;
    auto r = getLocalBounds();
    
    auto topBar = r.removeFromTop(topTextBarHeight);
    auto bottomBar = r.removeFromBottom(bottomBarHeight);
    auto leftPanel = r.removeFromLeft(getWidth()/2);
    auto rightPanel = r;
    
    int velSliderWidth =leftPanel.getWidth()/4;
    auto VelocitySliderLabelArea = leftPanel.removeFromTop(30);
    g.setFont(18.f);
    g.drawFittedText("V1", VelocitySliderLabelArea.removeFromLeft(velSliderWidth), Justification::centred, 1);
    g.drawFittedText("V2", VelocitySliderLabelArea.removeFromLeft(velSliderWidth), Justification::centred, 1);
    g.drawFittedText("Vio", VelocitySliderLabelArea.removeFromLeft(velSliderWidth), Justification::centred, 1);
    g.drawFittedText("Cel", VelocitySliderLabelArea.removeFromLeft(velSliderWidth), Justification::centred, 1);
    
    auto VelocitySliderArea = leftPanel.removeFromTop(sliderheight);
    slider1.setBounds(VelocitySliderArea.removeFromLeft(velSliderWidth));
    velTapper1.setBounds(VelocitySliderArea.removeFromLeft(velSliderWidth));
    velTapper2.setBounds(VelocitySliderArea.removeFromLeft(velSliderWidth));
    velTapper3.setBounds(VelocitySliderArea.removeFromLeft(velSliderWidth));
    
    auto TKnoiseSliderArea = leftPanel.removeFromTop(sliderwidth);
    g.setFont(15.f);
    g.drawFittedText("TkNoise:",TKnoiseSliderArea.removeFromLeft(velSliderWidth), Justification::left, 1);
    TKNoiseSlider1.setBounds(TKnoiseSliderArea.removeFromLeft(velSliderWidth));
    TKNoiseSlider2.setBounds(TKnoiseSliderArea.removeFromLeft(velSliderWidth));
    TKNoiseSlider3.setBounds(TKnoiseSliderArea.removeFromLeft(velSliderWidth));
    
    auto MNoiseSliderArea = leftPanel.removeFromTop(sliderwidth);
    g.drawFittedText("MNoise:",MNoiseSliderArea.removeFromLeft(velSliderWidth), Justification::left, 1);
    MNoiseSlider1.setBounds(MNoiseSliderArea.removeFromLeft(velSliderWidth));
    MNoiseSlider2.setBounds(MNoiseSliderArea.removeFromLeft(velSliderWidth));
    MNoiseSlider3.setBounds(MNoiseSliderArea.removeFromLeft(velSliderWidth));
    
    
    int alphaSliderWidth =rightPanel.getWidth()/4;
    auto alphaSliderLabelArea = rightPanel.removeFromTop(30);
    g.setFont(18.f);
    g.drawFittedText("V1", alphaSliderLabelArea.removeFromLeft(alphaSliderWidth), Justification::centred, 1);
    g.drawFittedText("V2", alphaSliderLabelArea.removeFromLeft(alphaSliderWidth), Justification::centred, 1);
    g.drawFittedText("Vio", alphaSliderLabelArea.removeFromLeft(alphaSliderWidth), Justification::centred, 1);
    g.drawFittedText("Cel", alphaSliderLabelArea.removeFromLeft(alphaSliderWidth), Justification::centred, 1);
    
    alphaMatrix.setBounds(rightPanel.removeFromTop(sliderheight + sliderwidth*2));
    
    filePathLabel.setBounds(bottomBar.removeFromLeft(getWidth()*0.8));
    filePathLabel.setText("MIDI: "+MidiFilePath, dontSendNotification);

    openButton.setBounds(bottomBar.reduced(10));

}


void MetroAudioProcessorEditor::resized()
{


}

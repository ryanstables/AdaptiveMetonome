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

class AlphaSquare    : public Component
{
public:
    AlphaSquare()
    {
        addRotarySlider(b11);
        addRotarySlider(b12);
        addRotarySlider(b13);
        addRotarySlider(b14);
        
        addRotarySlider(b21);
        addRotarySlider(b22);
        addRotarySlider(b23);
        addRotarySlider(b24);
        
        addRotarySlider(b31);
        addRotarySlider(b32);
        addRotarySlider(b33);
        addRotarySlider(b34);
        
        addRotarySlider(b41);
        addRotarySlider(b42);
        addRotarySlider(b43);
        addRotarySlider(b44);
    }
    
    ~AlphaSquare()
    {
    }
    
    
    void addRotarySlider(Slider &s)
    {
        // init the rotary TK/M noise sliders...
        addAndMakeVisible(s);
        s.setRange(0.0, 1.0, 0.0001);
        s.setValue(0.0);
        s.setSliderStyle(Slider::Rotary);
        // text box...
        s.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, s.getTextBoxWidth(), s.getTextBoxHeight());
        s.setTextBoxIsEditable(true);
        //s.addListener(this);
    }
    
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);
    }
    
    void resized() override
    {
        int width = getWidth(), height = getHeight(),
        numRows = 4, numCols = 4, indent = 2;
        
        auto r = getLocalBounds();
        auto row1 = r.removeFromTop(height/numRows);
        b11.setBounds(row1.removeFromLeft(width/numCols).reduced(indent));
        b12.setBounds(row1.removeFromLeft(width/numCols).reduced(indent));
        b13.setBounds(row1.removeFromLeft(width/numCols).reduced(indent));
        b14.setBounds(row1.reduced(indent));
        
        auto row2 = r.removeFromTop(height/numRows);
        b21.setBounds(row2.removeFromLeft(width/numCols).reduced(indent));
        b22.setBounds(row2.removeFromLeft(width/numCols).reduced(indent));
        b23.setBounds(row2.removeFromLeft(width/numCols).reduced(indent));
        b24.setBounds(row2.reduced(indent));
        
        auto row3 = r.removeFromTop(height/numRows);
        b31.setBounds(row3.removeFromLeft(width/numCols).reduced(indent));
        b32.setBounds(row3.removeFromLeft(width/numCols).reduced(indent));
        b33.setBounds(row3.removeFromLeft(width/numCols).reduced(indent));
        b34.setBounds(row3.reduced(indent));
        
        b41.setBounds(r.removeFromLeft(width/numCols).reduced(indent));
        b42.setBounds(r.removeFromLeft(width/numCols).reduced(indent));
        b43.setBounds(r.removeFromLeft(width/numCols).reduced(indent));
        b44.setBounds(r.reduced(indent));
    }
    
    Slider     b11{"1,1"}, b12{"1,2"}, b13{"1,3"}, b14{"1,4"},
    b21{"2,1"}, b22{"2,2"}, b23{"2,3"}, b24{"2,4"},
    b31{"3,1"}, b32{"3,2"}, b33{"3,3"}, b34{"3,4"},
    b41{"4,1"}, b42{"4,2"}, b43{"4,3"}, b44{"4,4"};
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlphaSquare);
};



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
    
    void addButton(Button &b);
    void addVelSlider(Slider &s);
    void addRotarySlider(Slider &s);


    // sliders to control the velocities of the tappers...
    Slider slider1, velTapper1, velTapper2, velTapper3;
    // sliders to control the noise of the synth tappers...
    Slider TKNoiseSlider1, TKNoiseSlider2, TKNoiseSlider3;
    Slider MNoiseSlider1,  MNoiseSlider2,  MNoiseSlider3;
    
    // the matrix of 4x4 alpha parmaters in a subcomponent
    AlphaSquare alphaMatrix;
    
    TextButton openButton;
    int numSynthTappers;
    
    Label filePathLabel;
    String MidiFilePath;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED

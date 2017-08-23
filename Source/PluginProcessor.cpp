/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Synth.h"

//==============================================================================
MetroAudioProcessor::MetroAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // read MIDI file into a stream...
    updateMIDIFile("/Users/ryanstables/Desktop/haydn.mid");
    printMIDIMessages();
    
    
    // update the piches in the tapManager based on the midi file...
    // currently there is a problem with this that needs to be fixed?!
    // I need to pass a pointer to inputMIDISeq, but the some of the other things, like adding channels mess up
    // tapManager->readPitchListFromMidiSeq(inputMIDISeq);
    
    
    // init the synth...
    const int numVoices = 8;
    for (int i = numVoices; --i >= 0;)
        synth.addVoice (new SineWaveVoice());
    synth.addSound (new SineWaveSound());
}

MetroAudioProcessor::~MetroAudioProcessor()
{
    
    
}

//==============================================================================
void MetroAudioProcessor::updateMIDIFile(String midiInputString)
{
    //clear MIDISeq buffers...
    for (int i=0; i<inputMIDISeq.size(); i++)
        inputMIDISeq[i]->clear();
    inputMIDISeq.clear();
    
    // create input stream from file...
    inputmidifile = midiInputString;
    FileInputStream MIDIData(inputmidifile);
    
    // create a MIDIFile object from the stream...
    MidiFile midiFileInput;
    midiFileInput.readFrom(MIDIData);
    
    // Create a sequence of MidiMessages...
    int numTracks = midiFileInput.getNumTracks();
    int activeTracknum = 0;
    for (int trackNum=0; trackNum<numTracks; trackNum++)
    {
        // iterate midimessages to check for noteOns...
        MidiMessageSequence currentTrack = *midiFileInput.getTrack(trackNum);
        int trackContainsNoteOnMessages = 0;
        for (int eventNum=0; eventNum<currentTrack.getNumEvents(); eventNum++)
        {
            if (currentTrack.getEventPointer(eventNum)->message.isNoteOn())
            {
                trackContainsNoteOnMessages++;
            }
        }
        
        // add the tracks to the inputMIDISeq array
        if(trackContainsNoteOnMessages)
        {
            inputMIDISeq.add(new MidiMessageSequence);
            inputMIDISeq[activeTracknum]->addSequence(currentTrack, 0.00);
            activeTracknum++;
        }
    }
}


void MetroAudioProcessor::printMIDIMessages()
{
    Logger::outputDebugString("File: "+inputmidifile.getFileName()+"..............");
    
    int numTracks = inputMIDISeq.size();
    for (int trackNum=0; trackNum < numTracks; trackNum++)
    {
        int activeNoteCounter = 0;
        int numEvents = inputMIDISeq[trackNum]->getNumEvents();
        for (int eventNum=0; eventNum<numEvents; eventNum++)
        {
            MidiMessageSequence::MidiEventHolder *tempEventHolder = inputMIDISeq[trackNum]->getEventPointer(eventNum);
            if(tempEventHolder->message.isNoteOn())
            {
                Logger::outputDebugString("x{"+String(trackNum+1)+"}("+String(activeNoteCounter+1)+") = "+String(inputMIDISeq[trackNum]->getEventTime(eventNum)));
                activeNoteCounter++;
            }
        }
    }
}

void MetroAudioProcessor::findAlignedMidiNotes()
{
    //only keep the midiMessages with N players performing simultaneously...
    
}


const String MetroAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MetroAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MetroAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double MetroAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MetroAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MetroAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MetroAudioProcessor::setCurrentProgram (int index)
{
}

const String MetroAudioProcessor::getProgramName (int index)
{
    return String();
}

void MetroAudioProcessor::changeProgramName (int index, const String& newName)
{
}




//==============================================================================
//========= Prepare to Play ====================================================
//==============================================================================
void MetroAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    if(!tappersAlreadyAllocated) // only allocate the tappers once
    {
        // WTF?!?!?!?! Why does the samplesPerBlock tell me 1024, when it should be 128???
        // this still needs to be fixed - how do we inherit the blocksize from the host?
        tapManager = new TapGenerator(numSynthesizedTappers+1, newSampleRate, /*samplesPerBlock*/ 128);
        tappersAlreadyAllocated = true;
    }

    synth.setCurrentPlaybackSampleRate (newSampleRate);
}

bool MetroAudioProcessor::bpmValueChanged()
{
    if(currentBPM!=prevBPM)
    {
        prevBPM = currentBPM;
        return true;
    }
    else
    {
        prevBPM = currentBPM;
        return false;
    }
}

//==============================================================================
//========= Process Block ======================================================
//==============================================================================
void MetroAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{

    // get bpm...
    getPlayHead()->getCurrentPosition(playhead);
    currentBPM = playhead.bpm;
    
    // do when plugin opens (we don't have access to an accurate bpm in prepareToPlay())
    if(globalCounter.inSamples() == 0 || bpmValueChanged())
        tapManager->updateBPM(currentBPM);
    
    getPlayHead()->getCurrentPosition(playhead);
    if (playhead.isPlaying)
    {
        
        // if the playhead is moving, start tapping...
        // Logger::outputDebugString("Frame: "+String(frameCounter.inSamples())+", playhead: "+String(playhead.timeInSamples));
        tapManager->nextBlock(midiMessages, globalCounter);
    }
    else
    {
        // clean up any left-over noteOns...
        tapManager->killActiveTappers(midiMessages);
    }
    
    // send the midi messages to the Synth...
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // counter++
    frameCounter.iterate();
}


//==============================================================================
//========= Release ============================================================
//==============================================================================
void MetroAudioProcessor::releaseResources()
{
    // write everything that left to the flie...

    
}






#ifndef JucePlugin_PreferredChannelConfigurations
bool MetroAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif









//==============================================================================
bool MetroAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MetroAudioProcessor::createEditor()
{
    return new MetroAudioProcessorEditor (*this);
}

//==============================================================================
void MetroAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MetroAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetroAudioProcessor();
}

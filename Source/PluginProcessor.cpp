// ------- Todo -----
//1...
// figure out the relationship between intervals and insets in matlab!
// - Writing the data to file causes audible distortion???? How do i fix this?

//2...
// - calculate the pertubations based on the input (write model function)
    // check for multiples, compensate for missing notes, etc
    // this will probably involve some kind of window

//3...
// warning: the beat len will change as it's based on the interval (with transofm added)
    // this could be changed to be static, but then runs the risk of being too long and may kill notes!?

//4...
// warning: need to make sure the last onset of the current beat happens before the one from the next beat starts
    // if not, the function that calculates the means will break!!!

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Synth.h"


//==============================================================================
AdaptiveTappersAudioProcessor::AdaptiveTappersAudioProcessor()
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
    // add the params to the proecessor...
    addParameter(FreqParam[0] = new AudioParameterFloat ("t1Freq", "Frequency of 1st tapper", 0.f, 127.f, 1.f));
    addParameter(DurParam[0] = new AudioParameterFloat  ("t1Dur", "Duration of 1st tapper", 1.f, 16.f, 1.f));
    addParameter(VelParam[0] = new AudioParameterFloat  ("t1Vel", "Velocity of 1st tapper", 0.f, 127.f, 1.f));
    
    addParameter(FreqParam[1] = new AudioParameterFloat ("t2Freq", "Frequency of 2nd tapper", 0.f, 127.f, 1.f));
    addParameter(DurParam[1] = new AudioParameterFloat  ("t2Dur", "Duration of 2nd tapper", 1.f, 16.f, 1.f));
    addParameter(VelParam[1] = new AudioParameterFloat  ("t2Vel", "Velocity of 2nd tapper", 0.f, 127.f, 1.f));
    
    addParameter(FreqParam[2] = new AudioParameterFloat ("t3Freq", "Frequency of 3rd tapper", 0.f, 127.f, 1.f));
    addParameter(DurParam[2] = new AudioParameterFloat  ("t3Dur", "Duration of 3rd tapper", 1.f, 16.f, 1.f));
    addParameter(VelParam[2] = new AudioParameterFloat  ("t3Vel", "Velocity of 3rd tapper", 0.f, 127.f, 1.f));
    
    addParameter(recordEnabled = new AudioParameterBool ("rec", "Enable Record mode", false));
    
    // add the voices and sounds to the synth object...
    const int numVoices = 16;
    // Add some voices...
    for (int i = numVoices; --i >= 0;)
        synth.addVoice (new SineWaveVoice());
    // ..and give the synth a sound to play
    synth.addSound (new SineWaveSound());
    
    // open the logfile/stream (this will be moved when i create a 'filename' text input
    File logFile ("/Users/ryanstables/Desktop/log.txt");
    logFile.appendText("in, t2, t3, t4 \n");
    captainsLog = new FileOutputStream (logFile);
    
    // prepare to start counting notes...
    noteCounter = 0;
}

AdaptiveTappersAudioProcessor::~AdaptiveTappersAudioProcessor()
{

}


//==============================================================================|
//====================== Tapper Functions ======================================|
//==============================================================================|

// Transform a tapping interval ----------------------------
int AdaptiveTappersAudioProcessor::transformIOI(int interval, double asynch)
{
    // TODO: also, make sure the mean-subtraction works for the first event too
    // ... when it gets implemented, it might need to be ignored as the means will be inited at 0?
    // ... this might not be an issue as the onset times are also 0?

        int randWinInSamples = (randWindowMs/1000.0)*fs;                                    // amount of randomness to be added
        int randomValue = rand.nextInt(randWinInSamples) - (randWinInSamples/2);            // generate scaled and offset random number
        Logger::outputDebugString("% "+String(randomValue));
        return (interval + randomValue) - asynch;                                           // return the interval with perturbation
                                                                                            // asynch = onset(n-1)-mean(n-1)
}


// Generate the synthesized tappers -----------------------------------------
void AdaptiveTappersAudioProcessor::generateTappers(MidiBuffer& midiMessages)
{
    bpm = playhead.bpm;                                                                 // get the BPM from the playhead (if it updates)
    interval = static_cast <int> (floor(60.f/bpm*fs));                                  // work out the interval inbetween note-on messages
    
    // init the first beat to the interval (this can't be done in the prepareToPlay() fn?)
    if(!beatCounter)
    {
        for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)                  // loop to synthesize the tappers
            synthesizedTapperInterval[tapperNum] = interval;
    }
    
    for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)                  // loop to synthesize the tappers
    {
        int synthTapperNum  = tapperNum+1;                                              // set MIDI channels for each tapper
        int channelNum      = tapperNum+2;                                              // set arrayPos for each tapper
                                                                                        // ...input is on channel 1, so first synth tapper is channel 2
        synthesizedTapperNoteLen[tapperNum] = round(interval/ *DurParam[tapperNum]);    // note lenth calculated as a beat division (in samples)
        
        for(int sampleNo=0; sampleNo<frameLen; sampleNo++)                              // loop over the samples in the current block...
        {
            //set NoteOn Messages --------
            int noteOnThresh = synthesizedTapperInterval[tapperNum]-1;
            if(noteOnCounter[tapperNum]<noteOnThresh)                                   // add noteOn events if the counter reaches the interval...
            {
                noteOnCounter[tapperNum]++;                                             // increment current noteOn counter
            }
            else
            {                                                                                       // NoteOn message at the current samplePosition...
                //apply transformation to the current interval...
                double asynchrony = currentTapTimes[synthTapperNum] - meanTapTime;
                synthesizedTapperInterval[tapperNum]   = transformIOI(interval, asynchrony);                    // add pertubation to the intervals
                
                if(synthesizedTapperNoteLen[tapperNum] >= synthesizedTapperInterval[tapperNum])     // to stop the note length exceeding the interval
                {
                    synthesizedTapperNoteLen[tapperNum] = synthesizedTapperInterval[tapperNum]-1;   // ...this shouldn't happen, but could if pertubations are high
                    Logger::outputDebugString("% Warning: interval shorter than note length on channel "+String(channelNum));
                }                                                                                   // ...as the lengths are based on the raw (not transformed) interval
                
                
                midiMessages.addEvent(MidiMessage::noteOn (channelNum, *FreqParam[tapperNum], static_cast <uint8> (*VelParam[tapperNum])),sampleNo);
                noteCurrentlyActive[tapperNum]=true;                                    // set noteActive flag so it can be removed when plyhead stops

                //register the time stamp and update the old one...
                prevTapTimes[synthTapperNum] = currentTapTimes[synthTapperNum];
                currentTapTimes[synthTapperNum] = ((frameLen * frameCounter) + sampleNo);   //onset time in samples (x/fs for seconds)
                channelHasNewValue[tapperNum] = true; //only has values for the synth tappers so far?

                noteOnCounter[tapperNum] = 0;                                               // reset notOn counter
                noteCounter++;                                                              // iterate note counter
            }
            
            //set NoteOff Messages --------
            int noteOffThresh = synthesizedTapperInterval[tapperNum]+synthesizedTapperNoteLen[tapperNum]-1;
            if(noteOffCounter[tapperNum]<=noteOffThresh)
            {
                noteOffCounter[tapperNum]++;                                            // increment noteOff counter if
            }
            else
            {
                midiMessages.addEvent(MidiMessage::allNotesOff(channelNum), sampleNo);          // add note off when the counter reaches the interval + noteLen...
                currentNoteOffTimes[synthTapperNum] = ((frameLen * frameCounter) + sampleNo);   //offset time in samples (x/fs for seconds)
                
                noteOffCounter[tapperNum] = synthesizedTapperNoteLen[tapperNum];                // reset noteOff counter to the noteLen
                noteCurrentlyActive[tapperNum]=false;                                           // cancel noteActive flag
            }
        }
    }
}

void AdaptiveTappersAudioProcessor::getMeanTaps()
{
    meanTapTime = (currentTapTimes[1]+currentTapTimes[2]+currentTapTimes[3])/3.0;     // calculate the mean syntheszied tap-times
    for(int i=0; i<numSynthesizedTappers; i++)                                                // reset the channelHasNewValue flags
        channelHasNewValue[i] = false;
}

// kill any active notes -----------------------------------------------------
void AdaptiveTappersAudioProcessor::killActiveTappers(MidiBuffer& midiMessages)
{
    for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)              // kill all of the synthesized tappers when the playhead stops...
    {
        if(noteCurrentlyActive[tapperNum])
        {
            int lastEventTime = midiMessages.getLastEventTime();                    // find the last tap time and add a note-off after it
            midiMessages.addEvent(MidiMessage::allNotesOff(tapperNum+2), lastEventTime+1);
            noteCurrentlyActive[tapperNum] = false;                                 // turn the active note flags off
        }
    }
}


bool AdaptiveTappersAudioProcessor::allTapsAreNew()
{
    if(channelHasNewValue[0] && channelHasNewValue[1] && channelHasNewValue[2])         // if all of the synth tappers have new values
        return true;
    else
        return false;
}



//============================= PREPARE ========================================|
void AdaptiveTappersAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // init fs and starting beat position...
    fs = sampleRate;
    frameLen = samplesPerBlock;
    beatPos = 0;
    
    // set samplerate for the synth...
    synth.setCurrentPlaybackSampleRate (sampleRate);
    
    
    bpm = playhead.bpm;                                                                 // get the BPM from the playhead
    interval = static_cast <int> (floor(60.f/bpm*fs));                                  // work out the interval inbetween note-on messages

    for(int tapperNum=0; tapperNum<numSynthesizedTappers; tapperNum++)                  // init the first interval, so the first event starts after 1 beat
        synthesizedTapperInterval[tapperNum] = interval;
        
    // prepare to start counting frames and notes...
    frameCounter = 0;
}


//==============================================================================|
//=========================== PROCESS BLOCK ====================================|
//==============================================================================|
void AdaptiveTappersAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    frameLen = buffer.getNumSamples(); // do I need to do this?

    // ------- generate synthesized tappers ----------------
    getPlayHead()->getCurrentPosition(playhead);
    if (playhead.isPlaying)
        generateTappers(midiMessages);                                                  // generate noteOn and noteOff messages for synthesized tapprers
    else
        killActiveTappers(midiMessages);                                                // kill the taps if the playhead stops before noteOffs are sent
    
    
    if(allTapsAreNew()) // when all of the taps have been updated
    {
        //set the current mean...
        getMeanTaps();
        // print some stuff to the log...
        String ons  = "Onsets("+String(beatCounter+1)+", :) \t\t=["+String(currentTapTimes[1])+", "+
                                                                    String(currentTapTimes[2])+", "+
                                                                    String(currentTapTimes[3])+
                                                                "]; ";
        String offs  = "Offsets("+String(beatCounter+1)+", :) \t\t=["+String(currentNoteOffTimes[1])+", "+
                                                                    String(currentNoteOffTimes[2])+", "+
                                                                    String(currentNoteOffTimes[3])+
                                                                "]; ";

        String ints = "Intervals("+String(beatCounter+1)+", :) \t=["+String(synthesizedTapperInterval[0])+", "+
                                                                     String(synthesizedTapperInterval[1])+", "+
                                                                     String(synthesizedTapperInterval[2])+
                                                                "];";
        String mns  = "Means("+String(beatCounter+1)+") \t\t\t="+String(meanTapTime)+String(";");
        Logger::outputDebugString(ints); Logger::outputDebugString(ons); Logger::outputDebugString(offs); Logger::outputDebugString(mns);
        beatCounter++;
    }
    
//    if(recordEnabled->get())                                                            // if the recording button is enabled
//    {
        // if prev frame was not recordenabled, reset all counters here!
        // ...
        
        // write data to log (when all new taps are registered)
        // ...
        // ...
        //    if (playhead.isPlaying)
        //    {
        //        if(!(frameCounter%100))
        //        {
        //            captainsLog->flush();                                                               // write the timings to the log file
        //        }
        //    }
//    }
    
    // ------- Trigger synth and write to log file --------
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());             // get our synth to process these midi events and generate its output.
    frameCounter++;                                                                     // ...always iterate the frame counter
}


//============================= RELEASE ========================================|
void AdaptiveTappersAudioProcessor::releaseResources()
{
    captainsLog->flush();                                                               // write the timings to the log file
}







//==============================================================================|

bool AdaptiveTappersAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AdaptiveTappersAudioProcessor::createEditor()
{
    return new AdaptiveTappersAudioProcessorEditor (*this);
}

void AdaptiveTappersAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AdaptiveTappersAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AdaptiveTappersAudioProcessor();
}

const String AdaptiveTappersAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AdaptiveTappersAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AdaptiveTappersAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double AdaptiveTappersAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AdaptiveTappersAudioProcessor::getNumPrograms()
{
    return 1;
}

int AdaptiveTappersAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AdaptiveTappersAudioProcessor::setCurrentProgram (int index)
{
}

const String AdaptiveTappersAudioProcessor::getProgramName (int index)
{
    return String();
}

void AdaptiveTappersAudioProcessor::changeProgramName (int index, const String& newName)
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AdaptiveTappersAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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


#pragma once

#include "engine/BaseProcessor.h"

namespace Element {

class AllPassFilterProcessor : public BaseProcessor
{
private:
    const bool stereo;
    AudioParameterFloat* length = nullptr;
    
public:
    /*
     // Freeverb tunings
     
     */
    explicit AllPassFilterProcessor (const bool _stereo = false)
    : BaseProcessor(), stereo (_stereo)
    {
        setPlayConfigDetails (stereo ? 2 : 1, stereo ? 2 : 1, 44100.0, 1024);
        addParameter (length   = new AudioParameterFloat ("length",   "Buffer Length",  1.f, 500.f, 90.f));
        lastLength = *length;
    }
    
    virtual ~AllPassFilterProcessor()
    {
        length = nullptr;
    }
    
    const String getName() const override { return "AllPass Filter"; }
    
    void fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = getName();
        desc.fileOrIdentifier   = stereo ? "element.allPass.stereo" : "element.allPass.mono";
        desc.descriptiveName    = stereo ? "AllPass Filter (stereo)" : "AllPass Filter (mono)";
        desc.numInputChannels   = stereo ? 2 : 1;
        desc.numOutputChannels  = stereo ? 2 : 1;
        desc.hasSharedContainer = false;
        desc.isInstrument       = false;
        desc.manufacturerName   = "Element";
        desc.pluginFormatName   = "Element";
        desc.version            = "1.0.0";
    }
    
    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        lastLength = *length;
        for (int i = 0; i < 2; ++i)
            allPass[i].setSize (*length * sampleRate * 0.001);
        setPlayConfigDetails (stereo ? 2 : 1, stereo ? 2 : 1,
                              sampleRate, maximumExpectedSamplesPerBlock);
    }
    
    void releaseResources() override
    {
        for (int i = 0; i < 2; ++i)
            allPass[i].free();
    }
    
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        if (lastLength != *length)
        {
            const int newSize = roundToIntAccurate (*length * getSampleRate() * 0.001);
            for (int i = 0; i < 2; ++i)
                allPass[i].setSize (newSize);
            lastLength = *length;
        }
        
        const int numChans = jmin (2, buffer.getNumChannels());
        const auto** input = buffer.getArrayOfReadPointers();
        auto** output = buffer.getArrayOfWritePointers();
        for (int c = 0; c < numChans; ++c)
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                output[c][i] = allPass[c].process (input[c][i]);
    }
    
    AudioProcessorEditor* createEditor() override   { return new GenericAudioProcessorEditor (this); }
    bool hasEditor() const override                 { return true; }
    
    double getTailLengthSeconds() const override    { return 0.0; };
    bool acceptsMidi() const override               { return false; }
    bool producesMidi() const override              { return false; }
    
    int getNumPrograms() override                                      { return 1; };
    int getCurrentProgram() override                                   { return 1; };
    void setCurrentProgram (int index) override                        { ignoreUnused (index); };
    const String getProgramName (int index) override                   { ignoreUnused (index); return "Parameter"; }
    void changeProgramName (int index, const String& newName) override { ignoreUnused (index, newName); }
    
    void getStateInformation (juce::MemoryBlock& destData) override
    {
        ValueTree state (Tags::state);
        state.setProperty ("length",   (float) *length, 0);
        if (ScopedPointer<XmlElement> e = state.createXml())
            AudioProcessor::copyXmlToBinary (*e, destData);
    }
    
    void setStateInformation (const void* data, int sizeInBytes) override
    {
        if (ScopedPointer<XmlElement> e = AudioProcessor::getXmlFromBinary (data, sizeInBytes))
        {
            auto state = ValueTree::fromXml (*e);
            if (state.isValid())
                *length   = (float) state.getProperty ("length",   (float) *length);
        }
    }
    
private:
    AllPassFilter allPass[2];
    float lastLength;
};

}


#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <memory>
#include <optional>
//#include <juce_events/broadcasters/juce_ChangeBroadcaster.h>

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor,
                        public juce::ChangeBroadcaster //adding change broadcasting to the processor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //function for loading multiple file into the mSampler
    void loadFiles();
    //function for clearing all loaded multiple files from loadedFiles array
    void clearFiles();
    //funtion to get names of files loaded by loadFiles
    juce::StringArray getLoadedFilesNames();
    
    // method to update index and notify listeners
    void setCurrentlyPlayingFileIndex(int newIndex);
    int getCurrentlyPlayingFileIndex() const;

    

private:
    juce::Synthesiser mSampler;
    const int mNumVoices { 32 };
    int currentlyPlayingFileIndex = -1;
    
    // Array to store loaded files
    std::vector<juce::File> loadedFiles;
    // Get random file index
    std::optional<int> getRandomFileIndex() const;
    // Get file at index
    std::optional<juce::File> getFileAtIndex (int index) const;

    // adding audio format manager
    juce::AudioFormatManager mFormatManager;
    juce::AudioFormatReader* mFormatReader { nullptr };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

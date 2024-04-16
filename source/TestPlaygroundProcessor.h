#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

#include <vector>
#include <memory>
#include <optional>

#include "ProcessorBase.h"

class TestPlaygroundProcessor : public ProcessorBase
{
public:
    TestPlaygroundProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer) override;

    // Sampler
    juce::Synthesiser mSampler;
    const int mNumVoices { 32 };
    int currentlyPlayingFileIndex = -1;
    
    // file chooser
    std::unique_ptr<juce::FileChooser> fileChooser;
    //function for loading multiple file into the mSampler
    void loadFiles();
    //function for clearing all loaded multiple files from loadedFiles array
    void clearFiles();
    //funtion to get names of files loaded by loadFiles
    juce::StringArray getLoadedFilesNames();
    
    // method to update index and notify listeners
    void setCurrentlyPlayingFileIndex(int newIndex);
    int getCurrentlyPlayingFileIndex() const;
    
    // Array to store loaded files
    std::vector<juce::File> loadedFiles;
    // Get random file index
    std::optional<int> getRandomFileIndex() const;
    // Get file at index
    std::optional<juce::File> getFileAtIndex (int index) const;

    // adding audio format manager
    juce::AudioFormatManager mFormatManager;
    
    // adding parameter values for pitch shift, offset and randomisation
    //    juce::AudioParameterFloat* pitchOffset;
    //    juce::AudioParameterFloat* pitchWheel;
    //    juce::AudioParameterFloat* randomPitchRange;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestPlaygroundProcessor)
};

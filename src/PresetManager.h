#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

/**
 * PresetManager - Handles preset save/load/browse
 *
 * Supports:
 * - Factory presets (built-in)
 * - User presets (saved to disk)
 */
class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts)
        : state(apvts)
    {
        initFactoryPresets();

        // Create user preset directory
        userPresetDir = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory)
            .getChildFile("SeshNx")
            .getChildFile("Sanguinova")
            .getChildFile("Presets");

        if (!userPresetDir.exists())
            userPresetDir.createDirectory();

        refreshPresetList();
    }

    // Get list of all presets (factory + user)
    juce::StringArray getPresetNames() const
    {
        juce::StringArray names;
        for (const auto& preset : factoryPresets)
            names.add(preset.name);
        for (const auto& file : userPresetFiles)
            names.add(file.getFileNameWithoutExtension());
        return names;
    }

    int getNumFactoryPresets() const { return static_cast<int>(factoryPresets.size()); }

    // Load preset by index
    bool loadPreset(int index)
    {
        if (index < 0)
            return false;

        if (index < static_cast<int>(factoryPresets.size()))
        {
            // Factory preset
            applyPresetData(factoryPresets[index].data);
            currentPresetName = factoryPresets[index].name;
            return true;
        }

        int userIndex = index - static_cast<int>(factoryPresets.size());
        if (userIndex < static_cast<int>(userPresetFiles.size()))
        {
            // User preset
            return loadPresetFromFile(userPresetFiles[userIndex]);
        }

        return false;
    }

    // Load preset by name
    bool loadPreset(const juce::String& name)
    {
        // Check factory presets
        for (size_t i = 0; i < factoryPresets.size(); ++i)
        {
            if (factoryPresets[i].name == name)
                return loadPreset(static_cast<int>(i));
        }

        // Check user presets
        for (size_t i = 0; i < userPresetFiles.size(); ++i)
        {
            if (userPresetFiles[i].getFileNameWithoutExtension() == name)
                return loadPreset(static_cast<int>(factoryPresets.size() + i));
        }

        return false;
    }

    // Save current state as user preset
    bool savePreset(const juce::String& name)
    {
        auto file = userPresetDir.getChildFile(name + ".xml");
        auto stateTree = state.copyState();
        auto xml = stateTree.createXml();

        if (xml != nullptr && xml->writeTo(file))
        {
            currentPresetName = name;
            refreshPresetList();
            return true;
        }
        return false;
    }

    // Delete a user preset
    bool deletePreset(const juce::String& name)
    {
        auto file = userPresetDir.getChildFile(name + ".xml");
        if (file.existsAsFile())
        {
            file.deleteFile();
            refreshPresetList();
            return true;
        }
        return false;
    }

    const juce::String& getCurrentPresetName() const { return currentPresetName; }

    void refreshPresetList()
    {
        userPresetFiles.clear();
        auto files = userPresetDir.findChildFiles(
            juce::File::findFiles, false, "*.xml");
        files.sort();
        for (const auto& f : files)
            userPresetFiles.push_back(f);
    }

    juce::File getUserPresetDirectory() const { return userPresetDir; }

private:
    struct FactoryPreset
    {
        juce::String name;
        std::map<juce::String, float> data;
    };

    void initFactoryPresets()
    {
        // Default / Init
        factoryPresets.push_back({
            "Init",
            {
                {"INPUT_Q", 0.5f},
                {"COLOR", 1000.0f},
                {"FILTER_MODE", 2.0f},  // BP
                {"DRIVE", 20.0f},
                {"OUTPUT_LP", 20000.0f},
                {"OUTPUT_GAIN", 0.0f},
                {"STAGE_2X", 0.0f},
                {"STAGE_5X", 0.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 100.0f}
            }
        });

        // Warm Saturation
        factoryPresets.push_back({
            "Warm Saturation",
            {
                {"INPUT_Q", 0.3f},
                {"COLOR", 800.0f},
                {"FILTER_MODE", 0.0f},  // LP
                {"DRIVE", 15.0f},
                {"OUTPUT_LP", 12000.0f},
                {"OUTPUT_GAIN", 0.0f},
                {"STAGE_2X", 0.0f},
                {"STAGE_5X", 0.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 70.0f}
            }
        });

        // Gritty Edge
        factoryPresets.push_back({
            "Gritty Edge",
            {
                {"INPUT_Q", 0.6f},
                {"COLOR", 2000.0f},
                {"FILTER_MODE", 2.0f},  // BP
                {"DRIVE", 28.0f},
                {"OUTPUT_LP", 15000.0f},
                {"OUTPUT_GAIN", -2.0f},
                {"STAGE_2X", 1.0f},
                {"STAGE_5X", 0.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 85.0f}
            }
        });

        // Heavy Crunch
        factoryPresets.push_back({
            "Heavy Crunch",
            {
                {"INPUT_Q", 0.5f},
                {"COLOR", 1500.0f},
                {"FILTER_MODE", 2.0f},  // BP
                {"DRIVE", 35.0f},
                {"OUTPUT_LP", 10000.0f},
                {"OUTPUT_GAIN", -3.0f},
                {"STAGE_2X", 1.0f},
                {"STAGE_5X", 1.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 100.0f}
            }
        });

        // Extreme Destruction
        factoryPresets.push_back({
            "Extreme Destruction",
            {
                {"INPUT_Q", 0.7f},
                {"COLOR", 3000.0f},
                {"FILTER_MODE", 1.0f},  // HP
                {"DRIVE", 40.0f},
                {"OUTPUT_LP", 8000.0f},
                {"OUTPUT_GAIN", -5.0f},
                {"STAGE_2X", 1.0f},
                {"STAGE_5X", 1.0f},
                {"STAGE_10X", 1.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 100.0f}
            }
        });

        // Subtle Tape
        factoryPresets.push_back({
            "Subtle Tape",
            {
                {"INPUT_Q", 0.4f},
                {"COLOR", 500.0f},
                {"FILTER_MODE", 0.0f},  // LP
                {"DRIVE", 8.0f},
                {"OUTPUT_LP", 18000.0f},
                {"OUTPUT_GAIN", 1.0f},
                {"STAGE_2X", 0.0f},
                {"STAGE_5X", 0.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 50.0f}
            }
        });

        // Bright Exciter
        factoryPresets.push_back({
            "Bright Exciter",
            {
                {"INPUT_Q", 0.8f},
                {"COLOR", 5000.0f},
                {"FILTER_MODE", 1.0f},  // HP
                {"DRIVE", 18.0f},
                {"OUTPUT_LP", 20000.0f},
                {"OUTPUT_GAIN", 2.0f},
                {"STAGE_2X", 0.0f},
                {"STAGE_5X", 0.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 40.0f}
            }
        });

        // Bass Thickener
        factoryPresets.push_back({
            "Bass Thickener",
            {
                {"INPUT_Q", 0.6f},
                {"COLOR", 200.0f},
                {"FILTER_MODE", 0.0f},  // LP
                {"DRIVE", 22.0f},
                {"OUTPUT_LP", 6000.0f},
                {"OUTPUT_GAIN", 0.0f},
                {"STAGE_2X", 1.0f},
                {"STAGE_5X", 0.0f},
                {"STAGE_10X", 0.0f},
                {"AUTO_GAIN", 1.0f},
                {"MIX", 60.0f}
            }
        });
    }

    void applyPresetData(const std::map<juce::String, float>& data)
    {
        for (const auto& [paramId, value] : data)
        {
            if (auto* param = state.getParameter(paramId))
            {
                param->setValueNotifyingHost(
                    param->convertTo0to1(value));
            }
        }
    }

    bool loadPresetFromFile(const juce::File& file)
    {
        auto xml = juce::XmlDocument::parse(file);
        if (xml != nullptr && xml->hasTagName(state.state.getType()))
        {
            state.replaceState(juce::ValueTree::fromXml(*xml));
            currentPresetName = file.getFileNameWithoutExtension();
            return true;
        }
        return false;
    }

    juce::AudioProcessorValueTreeState& state;
    juce::File userPresetDir;
    std::vector<FactoryPreset> factoryPresets;
    std::vector<juce::File> userPresetFiles;
    juce::String currentPresetName{"Init"};
};

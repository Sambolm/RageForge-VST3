cmake_minimum_required(VERSION 3.22)
project(RageForge VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Supply -DJUCE_DIR=/path/to/JUCE for an offline build, otherwise fetch JUCE.
if(DEFINED JUCE_DIR)
 add_subdirectory("${JUCE_DIR}" JUCE)
else()
 include(FetchContent)
 FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.9 GIT_SHALLOW TRUE)
 FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(RageForge
 COMPANY_NAME "Eternatones Productions"
 BUNDLE_ID "com.eternatones.rageforge"
 PLUGIN_MANUFACTURER_CODE Etns
 PLUGIN_CODE RgF1
 PRODUCT_NAME "RageForge"
 IS_SYNTH TRUE
 NEEDS_MIDI_INPUT TRUE
 NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE
 EDITOR_WANTS_KEYBOARD_FOCUS FALSE
 COPY_PLUGIN_AFTER_BUILD FALSE
 FORMATS VST3 Standalone
)
juce_generate_juce_header(RageForge)
target_sources(RageForge PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h Source/RageDSP.h)
target_compile_definitions(RageForge PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RageForge PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

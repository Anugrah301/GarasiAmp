cmake_minimum_required(VERSION 3.24)
project(GarasiAmp VERSION 0.1.0 LANGUAGES CXX)

option(GARASI_BUILD_PLUGIN "Build the JUCE VST3 plugin" ON)
option(GARASI_BUILD_TESTS "Build framework-independent DSP tests" ON)

if (GARASI_BUILD_TESTS)
    include(CTest)
    add_executable(garasi_dsp_tests Source/AmpEngine.cpp tests/dsp_tests.cpp)
    target_compile_features(garasi_dsp_tests PRIVATE cxx_std_17)
    add_test(NAME GarasiDspTests COMMAND garasi_dsp_tests)
endif()

if (GARASI_BUILD_PLUGIN)
    include(FetchContent)
    # Reproducible framework revision. To use a downloaded local JUCE checkout,
    # configure with -DFETCHCONTENT_SOURCE_DIR_JUCE=C:/path/to/JUCE
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.16-security
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)

    juce_add_plugin(GarasiAmp
        COMPANY_NAME "Garasi Audio"
        BUNDLE_ID "com.garasiaudio.garasiamp"
        PLUGIN_MANUFACTURER_CODE Gras
        PLUGIN_CODE Gamp
        FORMATS VST3
        VST3_CATEGORIES Fx Distortion
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PRODUCT_NAME "Garasi Amp"
    )
    juce_generate_juce_header(GarasiAmp)
    target_sources(GarasiAmp PRIVATE
        Source/AmpEngine.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
    )
    target_include_directories(GarasiAmp PRIVATE Source)
    target_compile_features(GarasiAmp PRIVATE cxx_std_17)
    target_compile_definitions(GarasiAmp PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    )
    target_link_libraries(GarasiAmp PRIVATE
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
    )
endif()

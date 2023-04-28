#define PLUG_NAME "PitchAnalyzer"
#define PLUG_MFR "KucaKun"
#define PLUG_VERSION_HEX 0x00010000
#define PLUG_VERSION_STR "1.0.0"
#define PLUG_UNIQUE_ID '6QfY'
#define PLUG_MFR_ID 'Acme'
#define PLUG_URL_STR "https://iplug2.github.io"
#define PLUG_EMAIL_STR "spam@me.com"
#define PLUG_COPYRIGHT_STR "Copyright 2020 Acme Inc"
#define PLUG_CLASS_NAME PitchAnalyzer

#define BUNDLE_NAME "PitchAnalyzer"
#define BUNDLE_MFR "KucaKun"
#define BUNDLE_DOMAIN "com"

#define SHARED_RESOURCES_SUBPATH "PitchAnalyzer"

#define PLUG_CHANNEL_IO "1-1"

#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 1
#define PLUG_DOES_MIDI_OUT 1
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 0
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 1000
#define PLUG_HEIGHT 858
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 0

#define AUV2_ENTRY PitchAnalyzer_Entry
#define AUV2_ENTRY_STR "PitchAnalyzer_Entry"
#define AUV2_FACTORY PitchAnalyzer_Factory
#define AUV2_VIEW_CLASS PitchAnalyzer_View
#define AUV2_VIEW_CLASS_STR "PitchAnalyzer_View"

#define AAX_TYPE_IDS 'IPWV'
#define AAX_TYPE_IDS_AUDIOSUITE 'IPWA'
#define AAX_PLUG_MFR_STR "Acme"
#define AAX_PLUG_NAME_STR "PitchAnalyzer\nIPEF"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 1

#define VST3_SUBCATEGORY "Fx"

#define APP_NUM_CHANNELS 1
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0

#define APP_SIGNAL_VECTOR_SIZE 64
#define BUFFER_SIZE 2048
#define FFT_SIZE BUFFER_SIZE/2
#define NUM_HARMONIC_PROD 3
#define PLOT_SIZE 512
#ifndef _TOOLS_H_
#define _TOOLS_H_
constexpr int _const_ceil(float num) { return (static_cast<float>(static_cast<int>(num)) == num) ? static_cast<int>(num) : static_cast<int>(num) + ((num > 0) ? 1 : 0); }
constexpr int h_s_l() { return _const_ceil(static_cast<float>(BUFFER_SIZE / 2) / static_cast<float>(NUM_HARMONIC_PROD)); }
#endif
#define HARMONIC_SMALLEST_LENGTH h_s_l()

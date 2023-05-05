#pragma once
#include "IPlug_include_in_plug_hdr.h"

//cpp
#include "float.h"
#include <complex>
#include <semaphore>

//iplug
//#include "IPlugAPP_host.h"
#include "circbuf.h"

//intel
#include "mkl.h"
#include "ipps.h"
#include "ippvm.h"

//my
#include "circ.h"

using namespace iplug;

const int kNumPresets = 3;

enum EParams {
    kGain = 0,
    kNumParams
};

enum EMsgTags {
    kMsgTagButton1 = 0,
    kMsgTagButton2 = 1,
    kMsgTagButton3 = 2,
    kMsgTagBinaryTest = 3
};

enum EControlTags {
    kCtrlTagMeter = 0,
};
struct RuntimeSettings {
    bool send_plots = false;
    double sound_threshold = 0.07;
};
class PitchAnalyzer final : public Plugin {
public:
    PitchAnalyzer(const InstanceInfo& info);

    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    void ProcessMidiMsg(const IMidiMsg& msg) override;
    void OnReset() override;
    void OnIdle() override;
    bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;
    void OnParamChange(int paramIdx) override;

private:
    //IPlugAPPHost* mPAPPHost = nullptr;
    DFTI_DESCRIPTOR_HANDLE hand;
    MKL_LONG fft(sample* x, const int buffer_size);
    void harmonic_product_spectrum(sample* fft_x, sample* hps_out, const int size);
    double auto_corr(sample* fft_x, const int size);
    double getFreq(sample* processed_x, int length, double mean);
    void PlotOnUi(int plotNum, sample* data, int count);
    int tests();
    float mLastPeak = 0.;
    double mHpsFreq = 0.;
    double mFftFreq = 0.;
    int lastSentPlotIndex = 0;
    int sentPlotNum = 0;
    WDL_TypedCircBuf<sample> buffer;
    std::binary_semaphore lock{ 0 };
    std::map<int, sample*> plots;
    RuntimeSettings conf;
};

#pragma once
//iplug
#include "IPlug_include_in_plug_hdr.h"
#include "IPlugAPP_host.h"

//intel
#include "mkl.h"
#include "ipps.h"
#include "ippvm.h"

//cpp
#include "float.h"
#include <complex>
#include <semaphore>
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

typedef std::complex<double> Complex;
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
    IPlugAPPHost* mPAPPHost = nullptr;
    DFTI_DESCRIPTOR_HANDLE hand;
    MKL_LONG fft(sample* x);
    double harmonic_product_spectrum(sample* x);
    void PlotOnUi(int plotNum, sample* data, int count);
    float mLastPeak = 0.;
    double mLastFreq = 0.;
    int lastSentPlotIndex = 0;
    int sentPlotNum = 0;
    double FREQ_BINS[APP_SIGNAL_VECTOR_SIZE];
    std::vector<sample> buffer;
    std::binary_semaphore lock{ 0 };
    std::map<int, sample*> plots;
};

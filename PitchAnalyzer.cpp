
#include "PitchAnalyzer.h"
#include "IPlug_include_in_plug_src.h"

PitchAnalyzer::PitchAnalyzer(const InstanceInfo& info)
    : Plugin(info, MakeConfig(kNumParams, kNumPresets))
    , hand() {
    mEditorInitFunc = [&]() {
#ifdef OS_WIN
        LoadFile(R"(D:\rep\iPlug2\Examples\PitchAnalyzer\resources\web\index.html)", nullptr);
#else
        LoadFile("index.html", GetBundleID());
#endif
        EnableScroll(false);
    };

    for (int i = 0; i < APP_SIGNAL_VECTOR_SIZE; i++) {
        FREQ_BINS[i] = i * GetSampleRate() / APP_SIGNAL_VECTOR_SIZE;
    }
}
MKL_LONG PitchAnalyzer::fft(Ipp64fc* x) {
    MKL_LONG status = DftiCreateDescriptor(&hand, DFTI_DOUBLE, DFTI_COMPLEX, 1, APP_SIGNAL_VECTOR_SIZE);
    status = DftiCommitDescriptor(hand);
    status = DftiComputeForward(hand, x);
    status = DftiFreeDescriptor(&hand);
    return status;
};
double PitchAnalyzer::harmonic_product_spectrum(sample* x) {
    /* Convert to imaginary */
    Ipp64fc fft_x[APP_SIGNAL_VECTOR_SIZE];
    MKL_LONG status = ippsRealToCplx_64f(x, NULL, fft_x, APP_SIGNAL_VECTOR_SIZE);

    /* Compute an FFT */
    status = fft(fft_x);
    sample fft_mag[APP_SIGNAL_VECTOR_SIZE];
    ippsReal_64fc(fft_x, fft_mag, APP_SIGNAL_VECTOR_SIZE);
    ippsAbs_64f_I(fft_mag, APP_SIGNAL_VECTOR_SIZE);

    /* Compute an HPS */
    sample y[HARMONIC_SMALLEST_LENGTH];
    memcpy(y, fft_mag, HARMONIC_SMALLEST_LENGTH * sizeof(sample)); // y = mag[:smallestLength].copy()
    for (int prod_i = 2; prod_i < NUM_HARMONIC_PROD + 1; prod_i++) {
        // multiply by every second, every third ... every num_harmonic_prod+1
        // y *= mag[::prod_i][:smallestLength]
        for (int i = 0; i < HARMONIC_SMALLEST_LENGTH; i++) {
            y[i] *= fft_mag[i * prod_i];
        }
    }

    /* Compute frequency */
    int max_y_index = cblas_icamax(APP_SIGNAL_VECTOR_SIZE, fft_mag, 1);
    return FREQ_BINS[max_y_index];
}

void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    memcpy(&outputs, &inputs, sizeof(inputs));
    sample x[APP_SIGNAL_VECTOR_SIZE];
    sample maxVal = 0.;
    int nan_ctr = 0;
    for (int i = 0; i < nFrames; i++) {
        if (std::isnan(inputs[0][i]) || std::isnan(inputs[1][i])) {
            x[i] = 0.;
            nan_ctr += 1;
            continue;
        }
        x[i] = (inputs[0][i] + inputs[1][i]) / 2.;
        maxVal += std::fabs(x[i]);
    }
    mLastPeak = static_cast<float>(maxVal / (sample)nFrames);
    if (mLastPeak > 0.001)
        mLastFreq = harmonic_product_spectrum(x);
}

void PitchAnalyzer::OnReset() {}

bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) { return false; }

void PitchAnalyzer::OnIdle() { SendControlValueFromDelegate(0, mLastFreq); }

void PitchAnalyzer::OnParamChange(int paramIdx) {}

void PitchAnalyzer::ProcessMidiMsg(const IMidiMsg& msg) {
    TRACE;

    msg.PrintMsg();
    SendMidiMsg(msg);
}

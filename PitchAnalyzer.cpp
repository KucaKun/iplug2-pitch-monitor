
#include "PitchAnalyzer.h"
#include "IPlug_include_in_plug_src.h"

PitchAnalyzer::PitchAnalyzer(const InstanceInfo& info)
    : Plugin(info, MakeConfig(kNumParams, kNumPresets))
    , mPAPPHost(reinterpret_cast<IPlugAPPHost*>(info.pAppHost))
    , hand(), plots() {
    mEditorInitFunc = [&]() {
#ifdef OS_WIN
        LoadFile(R"(D:\rep\iPlug2\Examples\PitchAnalyzer\resources\web\index.html)", nullptr);
#else
        LoadFile("index.html", GetBundleID());
#endif
        EnableScroll(false);
    };
    buffer.SetSize(BUFFER_SIZE);

}
MKL_LONG PitchAnalyzer::fft(sample* x) {
    Ipp64fc fft_x[BUFFER_SIZE];
    MKL_LONG status = ippsRealToCplx_64f(x, NULL, fft_x, BUFFER_SIZE);

    // remove mean from signal
    Ipp64fc mean;
    ippsMean_64fc(fft_x, BUFFER_SIZE, &mean);
    ippsSubC_64fc_I(mean, fft_x, BUFFER_SIZE);

    // Windowing function
    ippsWinKaiser_64fc_I(fft_x, BUFFER_SIZE, 100);

    // FFT
    status = DftiCreateDescriptor(&hand, DFTI_DOUBLE, DFTI_COMPLEX, 1, BUFFER_SIZE);
    status = DftiCommitDescriptor(hand);
    status = DftiComputeForward(hand, fft_x);
    status = DftiFreeDescriptor(&hand);

    // Magnitudes
    ippsReal_64fc(fft_x, x, FFT_SIZE);
    ippsAbs_64f_I(x, FFT_SIZE);
    ippsLn_64f_I(x, FFT_SIZE);

    sample mean_after;
    ippsMean_64f(x, BUFFER_SIZE, &mean_after);
    ippsSubC_64f_I(mean_after, x, BUFFER_SIZE);

    return status;
}
double PitchAnalyzer::harmonic_product_spectrum(sample* x) {

    /* Compute an FFT */
    auto status = fft(x);

    PlotOnUi(0, x, FFT_SIZE);

    /* Compute an HPS */
    sample y[HARMONIC_SMALLEST_LENGTH];
    memcpy(y, x, HARMONIC_SMALLEST_LENGTH * sizeof(sample)); // y = mag[:smallestLength].copy()
    for (int prod_i = 2; prod_i < NUM_HARMONIC_PROD + 1; prod_i++) {
        // multiply by every second, every third ... every num_harmonic_prod+1
        // y *= mag[::prod_i][:smallestLength]
        for (int i = 0; i < HARMONIC_SMALLEST_LENGTH; i++) {
            y[i] *= x[i * prod_i];
            if (isinf(y[i])) {
                y[i] = DBL_MAX;
            }
        }
    }

    /* Compute frequency */
    int max_y_index = cblas_idamax(HARMONIC_SMALLEST_LENGTH, y, 1);
    // i_interp = parabolic(hps, i_peak)[0]
    double freq = max_y_index * GetSampleRate() / BUFFER_SIZE;
    return freq;
}

void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    buffer.Add(inputs[0], nFrames);
    if (buffer.NbInBuf() == BUFFER_SIZE) {
        sample x[BUFFER_SIZE];
        buffer.Get(x, BUFFER_SIZE);
        buffer.Add(x, BUFFER_SIZE);
        mLastFreq = harmonic_product_spectrum(x);
    }
}

void PitchAnalyzer::OnReset() {}

bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) { return false; }

void PitchAnalyzer::OnIdle() {
    SendControlValueFromDelegate(0, mLastFreq);

    //sending plots
    if (plots.contains(sentPlotNum)) {
        for (int i = 0; i < PLOT_SIZE; i++) {
            auto ctrl = lastSentPlotIndex + (sentPlotNum + 1) * 1024;
            lock.acquire();
            SendControlValueFromDelegate(ctrl, plots[sentPlotNum][lastSentPlotIndex]);
            lock.release();
            if (++lastSentPlotIndex == PLOT_SIZE) {
                lastSentPlotIndex = 0;
                if (++sentPlotNum == plots.size()) {
                    sentPlotNum = 0;
                }
                break;
            }
        }
    }
}

void PitchAnalyzer::OnParamChange(int paramIdx) {}

void PitchAnalyzer::ProcessMidiMsg(const IMidiMsg& msg) {
    TRACE;

    msg.PrintMsg();
    SendMidiMsg(msg);
}

void PitchAnalyzer::PlotOnUi(int plotNum, sample* data, int count) {
    if (!plots.contains(plotNum)) {
        plots[plotNum] = new sample[PLOT_SIZE];
    }
    lock.try_acquire();
    int step = 1;
    if (count > PLOT_SIZE)
        step = count / PLOT_SIZE;
    for (int i = 0; i < PLOT_SIZE; i++) {
        plots[plotNum][i] = data[i * step];
    }
    lock.release();
}


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
    //sample x[APP_SIGNAL_VECTOR_SIZE];
    for (int i = 0; i < APP_SIGNAL_VECTOR_SIZE; i++) {
        FREQ_BINS[i] = i * GetSampleRate() / APP_SIGNAL_VECTOR_SIZE;
        //x[i] = i;
    }
    /*Ipp64fc fft_x[APP_SIGNAL_VECTOR_SIZE];
    MKL_LONG status = ippsRealToCplx_64f(x, NULL, fft_x, APP_SIGNAL_VECTOR_SIZE);
    status = fft(fft_x);*/

}
MKL_LONG PitchAnalyzer::fft(Ipp64fc* x) {
    MKL_LONG status = DftiCreateDescriptor(&hand, DFTI_DOUBLE, DFTI_COMPLEX, 1, APP_SIGNAL_VECTOR_SIZE);
    status = DftiCommitDescriptor(hand);
    status = DftiComputeForward(hand, x);
    status = DftiFreeDescriptor(&hand);
    return status;
}
double PitchAnalyzer::harmonic_product_spectrum(sample* x) {
    /* Convert to imaginary */
    Ipp64fc fft_x[APP_SIGNAL_VECTOR_SIZE];
    MKL_LONG status = ippsRealToCplx_64f(x, NULL, fft_x, APP_SIGNAL_VECTOR_SIZE);
    //ippsWinBlackman_64fc_I(fft_x, APP_SIGNAL_VECTOR_SIZE, -0.16);

    /* Compute an FFT */
    status = fft(fft_x);
    sample fft_mag[FFT_SIZE];
    //ippsMagnitude_64fc(fft_x, fft_mag, APP_SIGNAL_VECTOR_SIZE);
    ippsReal_64fc(fft_x, fft_mag, FFT_SIZE);
    ippsAbs_64f_I(fft_mag, FFT_SIZE);

    /*# because we are using half of FFT spectrum.
    s_mag = np.abs(sp) * 2 / np.sum(win)
    # Convert to dBFS
    s_dbfs = 20 * np.log10(s_mag / ref)*/
    sample dbfs[FFT_SIZE];
    ippsMulC_64f_I(2., fft_mag, FFT_SIZE);
    ippsLog10_64f_A26(fft_mag, dbfs, FFT_SIZE);
    ippsMulC_64f_I(20., dbfs, FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; i++) {
        if (isinf(dbfs[i])) {
            dbfs[i] = DBL_MAX;
        }
    }

    /* Compute frequency */
    int max_y_index = cblas_icamax(APP_SIGNAL_VECTOR_SIZE, dbfs, 1);
    return FREQ_BINS[max_y_index];

    /* Compute an HPS */
    sample y[HARMONIC_SMALLEST_LENGTH];
    memcpy(y, fft_mag, HARMONIC_SMALLEST_LENGTH * sizeof(sample)); // y = mag[:smallestLength].copy()
    for (int prod_i = 2; prod_i < NUM_HARMONIC_PROD + 1; prod_i++) {
        // multiply by every second, every third ... every num_harmonic_prod+1
        // y *= mag[::prod_i][:smallestLength]
        for (int i = 0; i < HARMONIC_SMALLEST_LENGTH; i++) {
            y[i] *= fft_mag[i * prod_i];
            if (isinf(y[i])) {
                y[i] = DBL_MAX;
            }
        }
    }
}

void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    sample x[APP_SIGNAL_VECTOR_SIZE];
    auto nanCtr = 0;
    for (int i = 0; i < APP_SIGNAL_VECTOR_SIZE; i++) {
        if (isnan(inputs[0][i])) {
            x[i] = 0;
            nanCtr += 1;
        }
        else {
            x[i] = inputs[0][i];
        }
    }
    PlotOnUi(0, x, sizeof(x));

    mLastFreq = harmonic_product_spectrum(x);
}

void PitchAnalyzer::OnReset() {}

bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) { return false; }

void PitchAnalyzer::OnIdle() {
    SendControlValueFromDelegate(0, mLastFreq);

    //sending plots
    if (plots.contains(sentPlotNum)) {
        for (int i = 0; i < 100; i++) {
            auto ctrl = lastSentPlotIndex + (sentPlotNum + 1) * 1024;
            lock.acquire();
            SendControlValueFromDelegate(ctrl, plots[sentPlotNum][lastSentPlotIndex]);
            lock.release();
            if (++lastSentPlotIndex == APP_SIGNAL_VECTOR_SIZE) {
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
void PitchAnalyzer::PlotOnUi(int plotNum, sample* data, int size) {
    lock.try_acquire();
    if (!plots.contains(plotNum)) {
        plots[plotNum] = new sample[size];
    }
    for (int i = 0; i < APP_SIGNAL_VECTOR_SIZE; i++) {
        plots[plotNum][i] = i;
    }
    lock.release();
    //memcpy(plots[plotNum], data, size);
}

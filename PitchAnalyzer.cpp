
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
    if (tests() != 0) {
        DBGMSG("TESTS FAILED");
        exit(1);
    }
}

// INTEL FFT IN PLACE
MKL_LONG PitchAnalyzer::fft(sample* x, const int buffer_size) {
    Ipp64fc* fft_x = (Ipp64fc*)malloc(buffer_size * sizeof(Ipp64fc));
    if (fft_x == NULL) {
        return 1;
    }
    MKL_LONG status = ippsRealToCplx_64f(x, NULL, fft_x, buffer_size);

    // remove mean from signal
    Ipp64fc mean;
    ippsMean_64fc(fft_x, buffer_size, &mean);
    ippsSubC_64fc_I(mean, fft_x, buffer_size);


    // Windowing function
    ippsWinHamming_64fc_I(fft_x, buffer_size);

    // FFT
    status = DftiCreateDescriptor(&hand, DFTI_DOUBLE, DFTI_COMPLEX, 1, buffer_size);
    status = DftiCommitDescriptor(hand);
    status = DftiComputeForward(hand, fft_x);
    status = DftiFreeDescriptor(&hand);

    const int fft_size = buffer_size / 2;
    // Magnitudes
    //ippsReal_64fc(fft_x, x, fft_size);
    ippsMagnitude_64fc(fft_x, x, fft_size);
    free(fft_x);

    return status;
}

// Frequency detection taken from:
// https://github.com/endolith/waveform_analysis/blob/master/waveform_analysis
// thank you very much kind mister


/* Compute an HPS*/
void PitchAnalyzer::harmonic_product_spectrum(sample* fft_x, sample* hps_out, const int size) {
    memcpy(hps_out, fft_x, size * sizeof(sample)); // y = mag[:smallestLength].copy()
    for (int prod_i = 2; prod_i < NUM_HARMONIC_PROD + 1; prod_i++) {
        // multiply by every second, every third ... every num_harmonic_prod+1
        // y *= mag[::prod_i][:smallestLength]
        for (int i = 0; i < size; i++) {
            hps_out[i] += fft_x[i * prod_i]; // x is decimated
        }
    }
}

/* Auto correlation */
double PitchAnalyzer::auto_corr(sample* fft_x, const int size) {
    return 1.;
    int buf_size = 0;
    int corr_size = 2 * size;
    Ipp8u* working_buffer;
    IppEnum fun_cfg = (IppEnum)(ippAlgAuto | ippsNormB);
    sample* corr = (sample*)malloc(corr_size * sizeof(sample));
    if (corr == NULL) {
        return -1.;
    }
    ippsAutoCorrNormGetBufferSize(size, size, ipp64f, fun_cfg, &buf_size);
    working_buffer = ippsMalloc_8u(buf_size);
    ippsAutoCorrNorm_64f(fft_x, size, corr, corr_size, fun_cfg, working_buffer);

    // find valley
    double d = 0;
    int i = 0;
    while (d <= 0 && ++i < corr_size) {
        d = corr[i] - corr[i - 1];
    }

    // gaussian
    int peak_after_valley = cblas_idamax(corr_size - i, corr + i, 1) + i;
    double nominator = log(corr[peak_after_valley + 1] / corr[peak_after_valley - 1]);
    double denominator = 2 * log(
        (corr[peak_after_valley] * corr[peak_after_valley]) /
        (corr[peak_after_valley - 1] * corr[peak_after_valley + 1])
    );
    double dm = nominator / denominator;
    double i_interpolated = (dm + peak_after_valley);
    free(corr);
    return GetSampleRate() / i_interpolated;
}

/* Compute frequency from processed x with gaussian interpolation
    If mean is given, frequency returned will be below 0 when signal size is lower than half of mean.
    src: http://www.add.ece.ufl.edu/4511/references/ImprovingFFTResoltuion.pdf
*/
double PitchAnalyzer::getFreq(sample* processed_x, int length, double mean = 0) {
    int max_index = cblas_idamax(length, processed_x, 1);
    if (processed_x[max_index] < mean / 2 && mean > 0) {
        return -1;
    }
    double nominator = log(processed_x[max_index + 1] / processed_x[max_index - 1]);
    double denominator = 2 * log(
        (processed_x[max_index] * processed_x[max_index]) /
        (processed_x[max_index - 1] * processed_x[max_index + 1])
    );
    double dm = nominator / denominator;
    double freq = (GetSampleRate() * (dm + max_index)) / BUFFER_SIZE;
    return freq;
}

void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    buffer.Add(inputs[0], nFrames);
    if (buffer.NbInBuf() == BUFFER_SIZE) {
        sample x[BUFFER_SIZE];
        buffer.Get(x, BUFFER_SIZE);
        buffer.Add(&x[nFrames], BUFFER_SIZE - nFrames);// shift right, works if nFrames is constant

        sample max;
        ippsMax_64f(x, BUFFER_SIZE, &max);

        fft(x, BUFFER_SIZE);
        auto mean = x[0];
        auto new_fft_freq = getFreq(x, FFT_SIZE, mean);
        if (new_fft_freq > 0 && max > conf.sound_threshold) {
            mFftFreq = new_fft_freq;
        }
        else {
            mFftFreq = -1;
        }
        x[0] = 0; // remove mean from signal for further calculations and plot
        PlotOnUi(0, x, FFT_SIZE);
    }
}

void PitchAnalyzer::OnReset() {}

bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) { return false; }

void PitchAnalyzer::OnIdle() {
    SendControlValueFromDelegate(0, mHpsFreq);
    SendControlValueFromDelegate(1, mFftFreq);

    //sending plots
    if (plots.contains(sentPlotNum) && conf.send_plots) {
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
    if (!conf.send_plots) {
        return;
    }
    if (!plots.contains(plotNum)) {
        plots[plotNum] = new sample[PLOT_SIZE];
    }
    lock.try_acquire();
    int amount = count;
    if (amount > 512)
        amount = 512;
    memcpy(plots[plotNum], data, amount * sizeof(sample));
    lock.release();
}

int PitchAnalyzer::tests() {

    WDL_TypedCircBuf<sample> buffer;
    buffer.SetSize(3);
    // fill buffer
    sample t[1] = { 2 };
    sample t2[1] = { 3 };
    buffer.Add(t, 1);
    buffer.Add(t, 1);
    buffer.Add(t2, 1);

    // get first time and refill
    sample r[3];
    buffer.Get(r, 3);

    // refill and get
    buffer.Add(r, 3);
    buffer.Add(t2, 1);

    sample r2[3];
    buffer.Get(r2, 3);
    DBGMSG("%f | %f | %f", r[0], r[1], r[2]);
    DBGMSG("%f | %f | %f", r2[0], r2[1], r2[2]);

    CircBuf<sample> b(3);
    b.add_elements(t, 1);
    b.add_elements(t, 1);
    b.add_elements(t2, 1);
    sample* test;
    test = b.get_buffer()->data();
    DBGMSG("%f | %f | %f", test[0], test[1], test[2]);
    b.add_elements(t2, 1);
    DBGMSG("%f | %f | %f", test[0], test[1], test[2]);



    return 0;
}
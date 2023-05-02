
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
    ippsWinKaiser_64fc_I(fft_x, buffer_size, 1);

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

    //ippsAbs_64f_I(x, fft_size);
    //ippsLn_64f_I(x, fft_size);
    //ippsAbs_64f_I(x, fft_size); // was minus, since we between 0 and 1

    x[0] = 0; // dc component

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
            hps_out[i] *= fft_x[i * prod_i]; // x is decimated
        }
    }
}

/* Compute frequency from processed x with gaussian interpolation*/
// src: http://www.add.ece.ufl.edu/4511/references/ImprovingFFTResoltuion.pdf
double PitchAnalyzer::getFreq(sample* processed_x, int length) {
    int max_index = cblas_idamax(length, processed_x, 1);
    double nominator = log(processed_x[max_index + 1] / processed_x[max_index - 1]);
    double denominator = 2 * log(processed_x[max_index] * processed_x[max_index]) /
        (processed_x[max_index - 1] * processed_x[max_index + 1]);
    double dm = nominator / denominator;
    double freq = (GetSampleRate() * (dm + max_index)) / BUFFER_SIZE;
    return freq;
}

void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    buffer.Add(inputs[0], nFrames);
    if (buffer.NbInBuf() == BUFFER_SIZE) {
        sample x[BUFFER_SIZE];
        buffer.Get(x, BUFFER_SIZE);
        buffer.Add(x, BUFFER_SIZE);

        fft(x, BUFFER_SIZE);
        mFftFreq = getFreq(x, FFT_SIZE);
        PlotOnUi(0, x, FFT_SIZE);

        sample hps[HARMONIC_SMALLEST_LENGTH];
        harmonic_product_spectrum(x, hps, HARMONIC_SMALLEST_LENGTH);
        mHpsFreq = getFreq(hps, HARMONIC_SMALLEST_LENGTH);
    }
}

void PitchAnalyzer::OnReset() {}

bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) { return false; }

void PitchAnalyzer::OnIdle() {
    SendControlValueFromDelegate(0, mHpsFreq);
    SendControlValueFromDelegate(1, mFftFreq);

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
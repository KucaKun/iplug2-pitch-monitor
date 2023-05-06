
#include "PitchAnalyzer.h"
#include "IPlug_include_in_plug_src.h"
static std::string get_index_path(PitchAnalyzer& analyzer) {
#ifdef OS_WIN
#ifdef _DEBUG
    return R"(D:\rep\iPlug2\Examples\PitchAnalyzer\resources\web\index.html)";
#endif 
#ifndef _DEBUG
    wchar_t path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)&get_index_path, &hm) == 0) {
        int ret = GetLastError();
        fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
        // Return or however you want to handle an error.
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
        int ret = GetLastError();
        fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
        // Return or however you want to handle an error.
    }
    std::wstring t = std::wstring(path);
    std::string index_path = std::string(t.begin(), t.end()) + std::string("\\..\\index.html");
    return index_path;
#endif // !_DEBUG
#else
    analyzer->LoadFile("index.html", GetBundleID());
#endif
}
PitchAnalyzer::PitchAnalyzer(const InstanceInfo& info)
    : Plugin(info, MakeConfig(kNumParams, kNumPresets))
    , hand(), plots() {

    SetChannelLabel(ERoute::kInput, 0, "Main L");
    SetChannelLabel(ERoute::kInput, 1, "Main R");
    SetChannelLabel(ERoute::kInput, 2, "SideChain L");
    SetChannelLabel(ERoute::kInput, 3, "SideChain R");


    mEditorInitFunc = [=]() {
        LoadFile(get_index_path(*this).c_str(), GetBundleID()); // in vst3 bundle
        EnableScroll(false);
    };
    mainBuffer.SetSize(BUFFER_SIZE);
    sideBuffer.SetSize(BUFFER_SIZE);
    if (tests() != 0) {
        DBGMSG("TESTS FAILED");
        exit(1);
    }
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
    ippsFree(working_buffer);
    return GetSampleRate() / i_interpolated;
}


void PitchAnalyzer::manipulate_buffer(WDL_TypedCircBuf<sample>* buffer, sample* inputs, double& output_freq, int plot_num, int nFrames) {
    buffer->Add(inputs, nFrames);
    if (buffer->NbInBuf() == BUFFER_SIZE) {
        sample block[BUFFER_SIZE];
        buffer->Get(block, BUFFER_SIZE);
        buffer->Add(&block[nFrames], BUFFER_SIZE - nFrames); // shift right, works if nFrames is constant

        sample max;
        ippsMax_64f(block, BUFFER_SIZE, &max);

        fft(block, BUFFER_SIZE);

        auto mean = block[0];
        auto fft_freq = getFreq(block, FFT_SIZE, mean);

        block[0] = 0; // remove mean from signal for further calculations and plot
        auto corr_freq = auto_corr(block, FFT_SIZE);

        if (max > conf.sound_threshold) {
            if (fft_freq < corr_freq) {
                output_freq = fft_freq;
            }
            else {
                output_freq = corr_freq;
            }
        }
        else {
            output_freq = -1;
        }
        PlotOnUi(plot_num, block, FFT_SIZE);
    }
}
void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    const int nChans = NOutChansConnected();
    for (int i = 0; i < 4; i++) {
        bool connected = IsChannelConnected(ERoute::kInput, i);
        if (connected != mInputChansConnected[i]) {
            mInputChansConnected[i] = connected;
        }
    }

#ifndef _DEBUG // no output on debug
    for (int s = 0; s < nFrames; s++) {
        for (int c = 0; c < nChans; c++) {
            outputs[c][s] = inputs[c][s];
        }
    }
#endif // _DEBUG

    if (mInputChansConnected[0]) {
        manipulate_buffer(&mainBuffer, inputs[0], mMainFreq, 0, nFrames);
    }
    if (mInputChansConnected[2]) {
        manipulate_buffer(&sideBuffer, inputs[2], mSideFreq, 1, nFrames);
    }
}

void PitchAnalyzer::GetBusName(ERoute direction, int busIdx, int nBuses, WDL_String& str) const {
    if (direction == ERoute::kInput) {
        if (busIdx == 0)
            str.Set("Main Input");
        else
            str.Set("SideChain");
    }
    else {
        str.Set("Output");
    }
}

void PitchAnalyzer::OnIdle() {
    SendControlValueFromDelegate(0, mMainFreq);
    SendControlValueFromDelegate(1, mSideFreq);

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


bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) { return false; }

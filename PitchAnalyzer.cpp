#include "PitchAnalyzer.h"
#include "IPlug_include_in_plug_src.h"

PitchAnalyzer::PitchAnalyzer(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitGain("Gain", -70., -70, 0.);

  // Hard-coded paths must be modified!
  mEditorInitFunc = [&]() {
#ifdef OS_WIN
    LoadFile(R"(D:\rep\iPlug2\Examples\PitchAnalyzer\resources\web\index.html)", nullptr);
#else
    LoadFile("index.html", GetBundleID());
#endif
    
    EnableScroll(false);
  };
  
  MakePreset("One", -70.);
  MakePreset("Two", -30.);
  MakePreset("Three", 0.);
}

void PitchAnalyzer::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->DBToAmp();
  
  sample maxVal = 0.;
  

  for (int s = 0; s < nFrames; s++)
  {
    outputs[0][s] = inputs[0][s] * mGainSmoother.Process(gain);
    outputs[1][s] = outputs[0][s]; // copy left
    
    maxVal += std::fabs(outputs[0][s]);
  }
  
  mLastPeak = static_cast<float>(maxVal / (sample) nFrames);

  // here do FFT and harmonic stuff and save it in mLastFreq  mLastNote

  /*
  FREQUENCIES = np.fft.fftfreq(WINDOW_SIZE, 1 / SAMPLE_RATE)[: WINDOW_SIZE // 2]
  def harmonic_product_spectrum(chunk):
      num_prod = 3
      mag = np.abs(np.fft.fft(chunk).real)[: WINDOW_SIZE // 2]
      smallestLength = int(np.ceil(len(mag) / num_prod))
      y = mag[:smallestLength].copy()
      for i in range(2, num_prod + 1):
          y *= mag[::i][:smallestLength]

      return (note float) 12 * np.log(FREQUENCIES[y.argmax()] / 440) / np.log(2)
    */
}

void PitchAnalyzer::OnReset()
{
  auto sr = GetSampleRate();
  mOscillator.SetSampleRate(sr);
  mGainSmoother.SetSmoothTime(20., sr);
}

bool PitchAnalyzer::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  if (msgTag == kMsgTagButton1)
    Resize(512, 335);
  else if(msgTag == kMsgTagButton2)
    Resize(1024, 335);
  else if(msgTag == kMsgTagButton3)
    Resize(1024, 768);
  else if (msgTag == kMsgTagBinaryTest)
  {
    auto uint8Data = reinterpret_cast<const uint8_t*>(pData);
    DBGMSG("Data Size %i bytes\n",  dataSize);
    DBGMSG("Byte values: %i, %i, %i, %i\n", uint8Data[0], uint8Data[1], uint8Data[2], uint8Data[3]);
  }

  return false;
}

void PitchAnalyzer::OnIdle()
{
    SendControlValueFromDelegate(kCtrlTagMeter, mLastPeak);
}

void PitchAnalyzer::OnParamChange(int paramIdx)
{
  DBGMSG("gain %f\n", GetParam(paramIdx)->Value());
}

void PitchAnalyzer::ProcessMidiMsg(const IMidiMsg& msg)
{
  TRACE;
  
  msg.PrintMsg();
  SendMidiMsg(msg);
}

#include "AudioTools.h"


uint16_t sample_rate=44100;
uint8_t channels = 2;                                      // The stream will have 2 channels 
SineWaveGenerator<int16_t> sineWave(32000);                // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave);             // Stream generated from sine wave
ResampleStream<int16_t> resample(sound);
CsvStream<int16_t> out(Serial); 
StreamCopy copier(out, resample);                        // copies sound to out

// Arduino Setup
void setup(void) {  
  // Open Serial 
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // define resample
  auto cfgr = resample.defaultConfig();
  cfgr.channels = channels;
  cfgr.sample_rate_from = sample_rate; 
  cfgr.sample_rate = 22050; 
  resample.begin(cfgr); 

  // Define CSV Output
  auto config = out.defaultConfig();
  config.sample_rate = sample_rate; 
  config.channels = channels;
  out.begin(config);

  // Setup sine wave
  sineWave.begin(channels, sample_rate, N_B4);
  Serial.println("started...");
}

// Arduino loop - copy sound to out 
void loop() {
  copier.copy();
}
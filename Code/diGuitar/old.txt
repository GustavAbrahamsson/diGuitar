#include <Arduino.h>
#include "AudioTools.h"

using namespace audio_tools;

#define volume 1000

#define pot1Pin A4

uint32_t pot1Value = 0;

typedef int16_t sound_t;                                   // sound will be represented as int16_t (with 2 bytes)
uint16_t sample_rate=44100;
uint8_t channels = 2;                                      // The stream will have 2 channels 
SineWaveGenerator<sound_t> sineWave(volume);                // subclass of SoundGenerator with max amplitude of 32000
SquareWaveGenerator<sound_t> squareWave(volume);

GeneratedSoundStream<sound_t> sound(sineWave);   // Stream generated from sine wave
GeneratedSoundStream<sound_t> sound2(squareWave);   // Stream generated from sine wave
I2SStream out; 
StreamCopy copier(out, sound2);                             // copies sound into i2s
StreamCopy copier2(out, sound2);      

// Arduino Setup
void setup(void) {
  // Open Serial 
  Serial.begin(115200);

  pinMode(pot1Pin, INPUT);

  // start the bluetooth
  Serial.println("starting I2S...");
  I2SConfig config = out.defaultConfig(TX_MODE);
  config.sample_rate = sample_rate; 
  config.channels = channels;
  config.bits_per_sample = 16;
  out.begin(config);

  // Setup sine wave
  //squareWave.begin(channels, sample_rate, 500);
}

// Arduino loop - copy sound to out 
void loop() {
  pot1Value = analogRead(pot1Pin);
  Serial.println(pot1Value);
  squareWave.begin(channels, sample_rate, map(pot1Value,0,4024,20,1000));
  
  copier.copy();
}
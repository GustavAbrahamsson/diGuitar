#include <Arduino.h>
#include "AudioTools.h"

using namespace audio_tools;

#define volume 150

#define pot1Pin A4

uint32_t pot1Value = 0;

typedef int16_t sound_t;                                   // sound will be represented as int16_t (with 2 bytes)
uint16_t sample_rate=44100;
uint8_t channels = 2;                                      // The stream will have 2 channels 
SineWaveGenerator<sound_t> sineWave(volume);                // subclass of SoundGenerator with max amplitude of 32000
SquareWaveGenerator<sound_t> squareWaveE(volume);
SquareWaveGenerator<sound_t> squareWaveA(volume);
SquareWaveGenerator<sound_t> squareWaveD(volume);

GeneratedSoundStream<sound_t> sound(sineWave);   // Stream generated from sine wave
GeneratedSoundStream<sound_t> soundE(squareWaveE);
GeneratedSoundStream<sound_t> soundA(squareWaveA);
GeneratedSoundStream<sound_t> soundD(squareWaveD);
I2SStream out; 
StreamCopy copier(out, sound);                          // copies sound into i2s
StreamCopy copierE(out, soundE);
//StreamCopy copierA(out, soundA);
StreamCopy copierD(out, soundD);      

const uint16_t E_STRING_FREQS[] = {82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262};
const uint16_t A_STRING_FREQS[] = {110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349};
const uint16_t D_STRING_FREQS[] = {147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466};
const uint16_t G_STRING_FREQS[] = {196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622};
const uint16_t B_STRING_FREQS[] = {247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784};
const uint16_t THIN_E_STRING_FREQS[] = {349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 933, 1047};


uint8_t noteIndex = 0;

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
  noteIndex = map(pot1Value, 0, 4024, 0, 20);
  Serial.println(noteIndex);
  //squareWave.begin(channels, sample_rate, E_STRING_FREQS[noteIndex]);

  squareWaveE.begin(channels, sample_rate, E_STRING_FREQS[noteIndex]);
  squareWaveA.begin(channels, sample_rate, A_STRING_FREQS[noteIndex+2]);
  //squareWaveD.begin(channels, sample_rate, D_STRING_FREQS[noteIndex+2]);
  
  copierE.copy();
  //copierA.copy();
  //copierD.copy();
}
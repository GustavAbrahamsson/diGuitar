#include <Arduino.h>
#include "AudioTools.h"
#include "Audiolibs/AudioKit.h"

using namespace audio_tools;

#define MAX_VOLUME 150
#define DEFAULT_BOOST_VOL 1.0

#define pot1Pin A4

uint32_t pot1Value = 0;

typedef int16_t sound_t;
uint16_t sample_rate = 44100;
uint8_t channels = 2;

SquareWaveGenerator<sound_t> squareWaveE(MAX_VOLUME);
SquareWaveGenerator<sound_t> squareWaveA(MAX_VOLUME);
SquareWaveGenerator<sound_t> squareWaveD(MAX_VOLUME);

//GeneratorMixer<SineWaveGenerator<int16_t>> mixer();

//GeneratedSoundStream<sound_t> sound(sineWave);
GeneratedSoundStream<sound_t> soundE(squareWaveE);
GeneratedSoundStream<sound_t> soundA(squareWaveA);
GeneratedSoundStream<sound_t> soundD(squareWaveD);
//StreamCopy copierE(out, soundE);
//StreamCopy copierA(out, soundA);
//StreamCopy copierD(out, soundD);

I2SStream i2s;

SineWaveGenerator<sound_t> sineWave(MAX_VOLUME);
AudioEffects<SineWaveGenerator<sound_t>> effects(sineWave);
GeneratedSoundStream<sound_t> effectSound(effects);
Boost boost(DEFAULT_BOOST_VOL);      

StreamCopy copier(i2s, effectSound);    

StreamCopy copier2(i2s, soundE);

float volFactor = 1.0;
ConverterScaler<float> volume(volFactor, 0, MAX_VOLUME);

const uint16_t E_STRING_FREQS[] =       {82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262};
const uint16_t A_STRING_FREQS[] =       {110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349};
const uint16_t D_STRING_FREQS[] =       {147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466};
const uint16_t G_STRING_FREQS[] =       {196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622};
const uint16_t B_STRING_FREQS[] =       {247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784};
const uint16_t THIN_E_STRING_FREQS[] =  {349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 933, 1047};

const uint16_t ALL_FREQS[] =            {73, 78, 82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 
                                        262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 
                                        880, 932, 933, 1047};
uint8_t noteIndex = 0;

String tuning = "E-standard";   // "Half step down", "D-standard"

void setup(void) {
  Serial.begin(115200);
  pinMode(pot1Pin, INPUT);
  Serial.println("starting I2S...");
  I2SConfig config = i2s.defaultConfig(TX_MODE);
  config.sample_rate = sample_rate; 
  config.channels = channels;
  config.bits_per_sample = 16;
  i2s.begin(config);

  sineWave.begin(channels, sample_rate, 500);
  sineWave.setFrequency(100);
  boost.setActive(1);
  boost.setVolume(0.5);
  effects.addEffect(boost);
  effectSound.begin(config);
  
  squareWaveE.begin(channels, sample_rate, 100);
}
 
void loop() {
  pot1Value = analogRead(pot1Pin);
  //Serial.println(pot1Value);
  noteIndex = map(pot1Value, 0, 4024, 0, 20);
  volFactor = (double)pot1Value/4050;
  //Serial.println(volFactor);

  //Serial.println(boost.volume());
  //squareWaveE.begin(channels, sample_rate, 100);


  
  //if(abs(boost.volume() - volFactor) > 0.02) boost.setVolume(volFactor);
  if(abs(volume.factor() - volFactor) > 0.05) volume.setFactor(volFactor);

  //Serial.println(noteIndex);
  //squareWave.begin(channels, sample_rate, E_STRING_FREQS[noteIndex]);

  //squareWaveE.begin(channels, sample_rate, E_STRING_FREQS[noteIndex]);
  //squareWaveA.begin(channels, sample_rate, A_STRING_FREQS[noteIndex+2]);
  //squareWaveD.begin(channels, sample_rate, D_STRING_FREQS[noteIndex+2]);

  //copier.copy();
  copier2.copy(volume);
  //copierA.copy();
  //copierD.copy();
}
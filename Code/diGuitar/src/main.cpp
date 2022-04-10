#include <Arduino.h>
#include "AudioTools.h"
#include "Audiolibs/AudioKit.h"

using namespace audio_tools;

#define MAX_VOLUME 32767

#define BOOST_VOLUME 2
#define FUZZ_EFFECT 5
#define DIST_CLIP 4990
#define DIST_MAX_IN 6500

#define pot1Pin A4
#define rib1Pin A5
#define btn1Pin 36

uint32_t pot1Value = 0;
uint32_t rib1Value = 0;
bool btn1On = 0;
bool fuzzGone = 0;

typedef int16_t sound_t;
uint16_t sample_rate = 44100;
uint8_t channels = 2;

unsigned long lastTime = 0;

SquareWaveGenerator<sound_t> squareWaveE(MAX_VOLUME);

//GeneratorMixer<SineWaveGenerator<int16_t>> mixer();

GeneratedSoundStream<sound_t> soundE(squareWaveE);

I2SStream i2s;

VolumeStream volumeE(soundE);

SineWaveGenerator<sound_t> sineWave(MAX_VOLUME);
AudioEffects<SineWaveGenerator<sound_t>> effects(sineWave);
GeneratedSoundStream<sound_t> effectSound(effects);

Boost boost(BOOST_VOLUME);      
Fuzz fuzz(FUZZ_EFFECT);
Distortion dist(DIST_CLIP, DIST_MAX_IN);
//StreamCopy copier(i2s, effectSound);  

GeneratedSoundStream<sound_t> sound(sineWave);

//VolumeStream volume2(effectSound);
VolumeStream volume2(sound);
LinearVolumeControl lvc;

StreamCopy copier2(i2s, volume2);

double volFactor = 0.1;

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

void setupGPIO(){
  pinMode(pot1Pin, INPUT);
  pinMode(rib1Pin, INPUT);
  pinMode(btn1Pin, INPUT);
  
}

void setup(void) {
  Serial.begin(115200);
  setupGPIO();
  Serial.println("starting I2S...");
  I2SConfig config = i2s.defaultConfig(TX_MODE);
  config.sample_rate = sample_rate; 
  config.channels = channels;
  config.bits_per_sample = 16;
  i2s.begin(config);

  sineWave.begin(channels, sample_rate, 500);
  sineWave.setFrequency(200);
  fuzz.setFuzzEffectValue(FUZZ_EFFECT);
  fuzz.setMaxOut(300);
  //effects.addEffect(fuzz);
  //effects.addEffect(boost);
  //effects.addEffect(dist);

  //effectSound.begin(config);
  //sound.begin(config);

  volume2.begin(config);
  volume2.setVolume(0.7);
  volume2.setVolumeControl(lvc);
}
 
void loop() {
  pot1Value = analogRead(pot1Pin);
  rib1Value = analogRead(rib1Pin);
  
  if(rib1Value > 100 && rib1Value < 5000) {
    rib1Value = map(rib1Value,300, 4095, 80, 600);
    sineWave.setFrequency(rib1Value);
    //Serial.println(rib1Value);
  }else{
    rib1Value = 0;
    //Serial.println(rib1Value);
    sineWave.setFrequency(80);
  }
  

  //btn1On = digitalRead(btn1Pin);
  // Serial.println(pot1Value);
  noteIndex = map(pot1Value, 0, 4024, 0, 20);
  volFactor = (double)pot1Value/4050;
  
  if (abs(volume2.volume() - volFactor) > 0.1){
    volume2.setVolume(volFactor);
  }
  copier2.copy();
}
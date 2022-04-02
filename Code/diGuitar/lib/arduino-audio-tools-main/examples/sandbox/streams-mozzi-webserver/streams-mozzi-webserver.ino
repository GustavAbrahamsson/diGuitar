/**
 * @file streams-mozzi-webserver.ino
 * @author Phil Schatzmann
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-mozzi-webserver/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 */
#include "AudioTools.h"
#include "AudioMozzi.h"
#include <Oscil.h>
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <mozzi_fixmath.h>
#include <EventDelay.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>



MozziStream mozzi;
AudioWAVServer server("ssid","password");

/// Copied from AMsynth.ino
#define CONTROL_RATE 64 // Hz, powers of 2 are most reliable

// audio oscils
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModDepth(COS2048_DATA);

// for scheduling note changes in updateControl()
EventDelay  kNoteChangeDelay;

// synthesis parameters in fixed point formats
Q8n8 ratio; // unsigned int with 8 integer bits and 8 fractional bits
Q24n8 carrier_freq; // unsigned long with 24 integer bits and 8 fractional bits
Q24n8 mod_freq; // unsigned long with 24 integer bits and 8 fractional bits

// for random notes
Q8n0 octave_start_note = 42;


void setup(){
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // configure mozzi
  auto cfg = mozzi.defaultConfig();
  cfg.channels = 1;
  cfg.updateAudio = updateAudio;
  cfg.updateControl = updateControl;
  mozzi.begin(cfg);
  
  // We start to the server  
  server.begin(mozzi, cfg.sample_rate, cfg.channels, cfg.bits_per_sample);

  // Start Mozzi
  ratio = float_to_Q8n8(3.0f);  // define modulation ratio in float and convert to fixed-point
  kNoteChangeDelay.set(200);    // note duration ms, within resolution of CONTROL_RATE
  aModDepth.setFreq(13.f);      // vary mod depth to highlight am effects
  randSeed(); // reseed the random generator for different results each time the sketch runs
}

void updateControl(){
  static Q16n16 last_note = octave_start_note;

  if(kNoteChangeDelay.ready()){

    // change octave now and then
    if(rand((byte)5)==0){
      last_note = 36+(rand((byte)6)*12);
    }

    // change step up or down a semitone occasionally
    if(rand((byte)13)==0){
      last_note += 1-rand((byte)3);
    }

    // change modulation ratio now and then
    if(rand((byte)5)==0){
      ratio = ((Q8n8) 1+ rand((byte)5)) <<8;
    }

    // sometimes add a fractionto the ratio
    if(rand((byte)5)==0){
      ratio += rand((byte)255);
    }

    // step up or down 3 semitones (or 0)
    last_note += 3 * (1-rand((byte)3));

    // convert midi to frequency
    Q16n16 midi_note = Q8n0_to_Q16n16(last_note);
    carrier_freq = Q16n16_to_Q24n8(Q16n16_mtof(midi_note));

    // calculate modulation frequency to stay in ratio with carrier
    mod_freq = (carrier_freq * ratio)>>8; // (Q24n8   Q8n8) >> 8 = Q24n8

      // set frequencies of the oscillators
    aCarrier.setFreq_Q24n8(carrier_freq);
    aModulator.setFreq_Q24n8(mod_freq);

    // reset the note scheduler
    kNoteChangeDelay.start();
  }
}

AudioOutput_t updateAudio(){
  int32_t mod = (128u+ aModulator.next()) * ((byte)128+ aModDepth.next());
  return MonoOutput::fromNBit(24, mod * aCarrier.next());
}

// Arduino loop  
void loop() {
  server.copy();
}
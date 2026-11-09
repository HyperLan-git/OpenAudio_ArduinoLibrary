#pragma once

#include "Arduino.h"
#include "AudioStream_F32.h"
#include "arm_math.h"
#include "mathDSP_F32.h"

constexpr int WAVETABLE_SIZE = 1024;
constexpr int BUFFER_SIZE = 512;

class ResynthYIN_F32 : public AudioStream_F32 {
  public:
    ResynthYIN_F32() : AudioStream_F32(1, inputs) {
        for(int i = 0; i < WAVETABLE_SIZE; i++) {
            wavetable[i] = ((float)i/WAVETABLE_SIZE);
            if(i > WAVETABLE_SIZE/2) wavetable[i] -= 1.f;
        }
    }

    void setWavetable(float32_t table[WAVETABLE_SIZE]) {
      arm_copy_f32(table, wavetable, WAVETABLE_SIZE);
    }

    float32_t getFreq();

  private:
    virtual void update();

    double phase = 0;

    float32_t freq = 0;

    audio_block_f32_t* inputs[1];
    
    float32_t buffer[BUFFER_SIZE];

    float32_t wavetable[WAVETABLE_SIZE];
};
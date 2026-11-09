#pragma once

#include "Arduino.h"
#include "AudioStream_F32.h"
#include "arm_math.h"
#include "mathDSP_F32.h"

constexpr int WAVETABLE_SIZE = 1024;

class ResynthYIN_F32 : public AudioStream_F32 {
  public:
    ResynthYIN_F32() : AudioStream_F32(1, inputs) {
        for(int i = 0; i < WAVETABLE_SIZE; i++) {
            wavetable[i] = i > WAVETABLE_SIZE/2 ? 1.f : 0.f;
        }
    }

  private:
    virtual void update();

    float32_t phase = 0;

    float32_t freq = 0;

    audio_block_f32_t* inputs[1];

    f32 wavetable[WAVETABLE_SIZE];
};
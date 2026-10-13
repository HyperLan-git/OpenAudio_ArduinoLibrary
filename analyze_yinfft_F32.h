/*
 * analyze_yinfft.h  Created 2026 HyperLan
 * for the OpenAudio_TeensyArduino library (floating point audio).
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

// ***************  TEENSY 4.X ONLY   ****************
// Use 1024 size, or smaller, for Teensy 3.x
#if defined(__IMXRT1062__)

#include "Arduino.h"
#include "AudioStream_F32.h"
#include "arm_math.h"
#include "mathDSP_F32.h"
#include "arm_const_structs.h"

constexpr int FFT_SIZE = 2048, FFT_D2 = FFT_SIZE/2;

constexpr float FFT_PI = 3.14159265359f;
constexpr float FFT_2PI = 6.28318530718f;

class AudioAnalyzeYINFFT_F32 : public AudioStream_F32
{
    public:
        AudioAnalyzeYINFFT_F32() {
            result[0] = 1;
            useHanningWindow();
            Sfft = arm_cfft_sR_f32_len2048;
        }
        
        void doFFT();
        void fillBlocksAndDoFFT();

        virtual void update(void);

        float getProbability() {
            return p;
        }

        float getFrequency() {
            return freq;
        }

        
    private:
        float sampleRate = AUDIO_SAMPLE_RATE;

        float fftData[FFT_SIZE];
        float fftIn[FFT_SIZE];
        float fftWindow[FFT_SIZE];

        float XS[FFT_D2];
        float result[FFT_D2];

        float freq = 0;
        float p = 0;

        // The Hann window is a good all-around window
        void useHanningWindow(void) {
            for (int i = 0; i < FFT_SIZE; i++) {
                // 2*PI/2047 = 0.00306946
                fftWindow[i] = 0.5*(1.0 - cosf(FFT_2PI*(float)i / (FFT_SIZE-1)));
            }
        }
};

#endif
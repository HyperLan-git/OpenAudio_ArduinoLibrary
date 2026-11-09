#include "resynthYIN_F32.h"

typedef float32_t f32;

constexpr int BUFSZ = 256;

f32 yinDifference[BUFSZ];
f32 yinCMND[BUFSZ];

constexpr float MINFREQ = 150;
constexpr float MAXFREQ = 1500;

f32 measurePitch(f32 *data, int samples, float sr, float threshold) {
    int tauMax = std::min(samples, BUFSZ * 2) / 2;

    f32 temp_delta[BUFSZ]; 

    // 1. Difference function
    for (int tau = 1; tau < tauMax; tau++) {
        // 1. temp_delta[i] = data[i] - data[i + tau]
        arm_sub_f32(&data[0], &data[tau], temp_delta, tauMax);
        
        // 2. sum = temp_delta[0]^2 + temp_delta[1]^2 + ...
        float sum = 0;
        arm_dot_prod_f32(temp_delta, temp_delta, tauMax, &sum);
        
        yinDifference[tau] = sum;
    }

    // 2. Cumulative mean normalized difference
    yinCMND[0] = 1;
    // d = sum from 0 to i of diff
    float runningSum = 0;

    for (int tau = 1; tau < tauMax; tau++) {
        runningSum += yinDifference[tau];
        yinCMND[tau] = yinDifference[tau] * tau / runningSum;
    }

    // 3. Absolute threshold

    // Min 150hz
    for (int tau = 2; tau < tauMax; tau++) {
        if (yinCMND[tau] < threshold) {
            // refine (optional)
            while (tau + 1 < tauMax && yinCMND[tau + 1] < yinCMND[tau]) {
                tau++;
            }

            float freq = sr / (float)tau;
            if (freq < MINFREQ) continue;
            if (freq > MAXFREQ) continue;
            return freq;
        }
    }

    return -1; // no pitch found

}

void ResynthYIN_F32::update() {
	audio_block_f32_t *block = AudioStream_F32::receiveReadOnly_f32();

    if(!block) return;

    f32 sr = block->fs_Hz;
    int samples = block->length;

    bool compute = true;
    if(compute) freq = measurePitch(block->data, samples, sr, 0.1);

	release(block);
    
    audio_block_f32_t *out_block = allocate_f32();
    if (!out_block)  {
        release(block);
        return;
    }

    const float dphase = (freq) / sr;
    for (int i = 0; i < samples; i++) {
        out_block->data[i] = phase > 0.5f ? 1.0f : -1.0f;

        phase += dphase;
        if (phase >= 1.0) phase -= 1.0f;
    }
    
	transmit(out_block);
	release(out_block);
}
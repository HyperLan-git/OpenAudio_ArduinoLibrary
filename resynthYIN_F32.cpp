#include "resynthYIN_F32.h"

typedef float32_t f32;

constexpr int BUFSZ = BUFFER_SIZE/2;

f32 yinDifference[BUFSZ];
f32 yinCMND[BUFSZ];
f32 temp_delta[BUFSZ];

constexpr float MINFREQ = 150;
constexpr float MAXFREQ = 1500;

f32 measurePitch(f32 *data, int samples, float sr, float threshold) {
    int tauMax = std::min(samples, BUFFER_SIZE) / 2; 

    // 1. Difference function
    for (int tau = 1; tau < tauMax; tau++) {
        // 1. temp_delta[i] = data[i] - data[i + tau]
        arm_sub_f32(data, data + tau, temp_delta, tauMax);
        
        // 2. sum = temp_delta[0]^2 + temp_delta[1]^2 + ...
        float sum = 0;
        arm_dot_prod_f32(temp_delta, temp_delta, tauMax, &sum);
        
        yinDifference[tau] = sum;
    }

    // 2. Cumulative mean normalized difference
    yinCMND[0] = 1;
    // d = sum from 0 to i of diff
    float runningSum = 0.00001f;

    for (int tau = 1; tau < tauMax; tau++) {
        runningSum += yinDifference[tau];
        yinCMND[tau] = yinDifference[tau] * tau / runningSum;
    }

    // 3. Absolute threshold

    // Min 150hz
    for (int tau = 2; tau < tauMax; tau++) {
        if (yinCMND[tau] < threshold) {
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

const float log440 = log2f(440.0f);

float freqToMidi(float freq)
{
    return 69.0f + 12.0f * (log2f(freq) - log440);
}

// x = 69 + 12 * log2(y / 440)
// (x - 69)/12 = log2(y)-log2(440)
// 2^((x-69)/12) * 440 = y

float midiToFreq(int midi)
{
    return exp2f((midi-69) / 12.f) * 440;
}

f32 oldrms = 0;

int num = 0;

void ResynthYIN_F32::update() {
	audio_block_f32_t *block = AudioStream_F32::receiveReadOnly_f32();

    if(!block) return;

    f32 sr = block->fs_Hz;
    int samples = block->length;

    arm_copy_f32(buffer + samples, buffer, BUFFER_SIZE - samples);
    arm_copy_f32(block->data, buffer + BUFFER_SIZE - samples, samples);

    bool compute = (++num %= 3) == 0;
    if(compute) {
        float f = measurePitch(buffer, BUFFER_SIZE, sr, 0.3);
        if(f != -1) freq = midiToFreq(roundf(freqToMidi(f)));
    }
    //++idx;
    //freq = block->data[0];

    f32 rms = 0;
    arm_dot_prod_f32(block->data, block->data, samples, &rms);
    //rms = sqrtf(rms / samples);
    rms /= samples;
    if(rms < 0.001) rms = 0;

	release(block);
    
    audio_block_f32_t *out_block = allocate_f32();
    if (!out_block)  {
        return;
    }

    const float dphase = (freq) / sr;
    const float damp = (rms - oldrms) / samples;
    for (int i = 0; i < samples; i++) {
        float idx = phase * (WAVETABLE_SIZE - 1);
        float nextSample = (int)(idx+1) < samples ? wavetable[(int)(idx+1)] : wavetable[0];
        float sample = wavetable[(int)(idx)];
        float prog = idx - floorf(idx);
        out_block->data[i] = sample * (1-prog) + nextSample * prog;
        out_block->data[i] *= oldrms + i * damp;

        phase += dphase;
        if (phase >= 1.0) phase -= 1.0f;
    }

    oldrms = rms;
    
	transmit(out_block);
	release(out_block);
}

float32_t ResynthYIN_F32::getFreq() {return freq;}
#include "analyze_yinfft_F32.h"

void AudioAnalyzeYINFFT_F32::doFFT() {
    // Teensyduino core for T4.x supports arm_cfft_f32
    // arm_cfft_f32 (const arm_cfft_instance_f32 *S, float32_t *p1, uint8_t ifftFlag, uint8_t bitReverseFlag)
    arm_cfft_f32 (&Sfft, fftIn, 0, 1);
}

static void apply_window_to_fft_buffer(void *buffer, const void *window) {
    float *buf = (float *)buffer;
    const float *win = (float *)window;

    for(int i=0; i < FFT_SIZE; i++)
       buf[i] *= *win++;
}

void AudioAnalyzeYINFFT_F32::update(void)  {
    audio_block_f32_t *block;

    block = AudioStream_F32::receiveReadOnly_f32();
    if (!block) return;

    memcpy(fftData, fftData + block->length, FFT_SIZE - block->length);
    memcpy(fftData + FFT_SIZE - block->length, block->data, FFT_SIZE - block->length);

    AudioStream_F32::release(block);

    // We could return here and update values only on every few blocks
    if(blocksToSkip > ++current) return;

    current = 0;
    {
        memcpy(fftIn, fftData, FFT_SIZE);
        apply_window_to_fft_buffer(fftIn, fftWindow);

        doFFT();
    }

    // TODO try to use less memory
    float sumA = 0;
    for(int i = 0; i < FFT_D2; i++) {
        float r = fftIn[i*2], im = fftIn[i*2+1];
        XS[i] = r*r + im*im;
        sumA += XS[i];
    }

    float sumres = 1;
    float N = FFT_SIZE;

    float min = 1;
    int minI = 0;
    for(int i = 1; i < FFT_D2; i++) {
        float sumB = 0;
        for(int k = 0; k < FFT_D2; k++)
            sumB += XS[k] * cosf(FFTYIN_2PI * k * i / N);

        result[i] = 4.0 / N * sumA - 2.0 / N * sumB;
        sumres = sumres + result[i];
        result[i] = result[i] * i / sumres;
        if(result[i] < min) {
            min = result[i];
            minI = i;
        }
    }
    p = -1;
    freq = sampleRate / minI;
    p = fabs(2 - min*2);
}

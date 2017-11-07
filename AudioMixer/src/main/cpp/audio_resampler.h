//
// Created by Piasy on 04/11/2017.
//

#ifndef AUDIOMIXER_AUDIO_SAMPLER_H
#define AUDIOMIXER_AUDIO_SAMPLER_H


#include <memory>

#include "avx_helper.h"

class AudioResampler {
public:
    AudioResampler(int bytesPerSample, int inChannelNum, int inSampleRate, int inSamples,
                   int outChannelNum, int outSampleRate);

    ~AudioResampler();

    uint8_t* getInputBuffer();

    int resample(const void* inData, int inLen, void* outData);

private:

    std::unique_ptr<SwrContext, SwrContextDeleter> context;
    uint8_t** inBuf;
    uint8_t** outBuf;
    AVSampleFormat fmt;
    int inSampleRate;
    int inChannelNum;
    int inSamples;
    int outSampleRate;
    int outChannelNum;
    int outSamples;
    int outLineSize;
};


#endif //AUDIOMIXER_AUDIO_SAMPLER_H

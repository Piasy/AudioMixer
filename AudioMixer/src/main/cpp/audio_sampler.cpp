//
// Created by Piasy on 04/11/2017.
//

#include "audio_sampler.h"

AudioSampler::AudioSampler(int inChannelNum, int inSampleRate, int outChannelNum, int outSampleRate)
        : context(swr_alloc(), SwrContextDeleter()) {
}

AudioSampler::~AudioSampler() {
}

int AudioSampler::resample(void* inData, int inLen, void* outData, int outLen) {
    return 0;
}

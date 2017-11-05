//
// Created by Piasy on 04/11/2017.
//

#ifndef AUDIOMIXER_AUDIO_SAMPLER_H
#define AUDIOMIXER_AUDIO_SAMPLER_H


#include <memory>

extern "C" {
#include <libswresample/swresample.h>
}

class AudioSampler {
public:
    AudioSampler(int inChannelNum, int inSampleRate, int outChannelNum, int outSampleRate);
    ~AudioSampler();

    int resample(void* inData, int inLen, void* outData, int outLen);

private:
    struct SwrContextDeleter {
        void operator() (SwrContext* swrContext) {
            swr_free(&swrContext);
        }
    };

    std::unique_ptr<SwrContext, SwrContextDeleter> context;
};


#endif //AUDIOMIXER_AUDIO_SAMPLER_H

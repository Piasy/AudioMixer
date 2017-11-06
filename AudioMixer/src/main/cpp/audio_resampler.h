//
// Created by Piasy on 04/11/2017.
//

#ifndef AUDIOMIXER_AUDIO_SAMPLER_H
#define AUDIOMIXER_AUDIO_SAMPLER_H


#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif

class AudioResampler {
public:
    AudioResampler(int inChannelNum, int inSampleRate, int inSamples, int outChannelNum,
                   int outSampleRate);

    ~AudioResampler();

    int resample(const void* inData, int inLen, void* outData);

private:
    struct SwrContextDeleter {
        void operator()(SwrContext* swrContext) {
            swr_free(&swrContext);
        }
    };

    std::unique_ptr<SwrContext, SwrContextDeleter> context;
    uint8_t** inBuf;
    uint8_t** outBuf;
    int inSampleRate;
    int inChannelNum;
    int inSamples;
    int outSampleRate;
    int outChannelNum;
    int maxOutSamples;
    int outLineSize;
};


#endif //AUDIOMIXER_AUDIO_SAMPLER_H

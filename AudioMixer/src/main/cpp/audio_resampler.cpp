//
// Created by Piasy on 04/11/2017.
//

#include <stdexcept>

#include <rtc_base/logging.h>

#include "audio_resampler.h"

AudioResampler::AudioResampler(int inChannelNum, int inSampleRate, int inSamples,
                               int outChannelNum, int outSampleRate)
        : context(swr_alloc(), SwrContextDeleter()),
          inBuf(nullptr),
          outBuf(nullptr),
          inSampleRate(inSampleRate),
          inChannelNum(inChannelNum),
          inSamples(inSamples),
          outSampleRate(outSampleRate),
          outChannelNum(outChannelNum) {
    if (!context.get()) {
        throw std::runtime_error("swr_alloc fail");
    }

    int64_t inChannelLayout = (inChannelNum == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    int64_t outChannelLayout = (outChannelNum == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;

    av_opt_set_int(context.get(), "in_channel_layout", inChannelLayout, 0);
    av_opt_set_int(context.get(), "in_sample_rate", inSampleRate, 0);
    av_opt_set_sample_fmt(context.get(), "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    av_opt_set_int(context.get(), "out_channel_layout", outChannelLayout, 0);
    av_opt_set_int(context.get(), "out_sample_rate", outSampleRate, 0);
    av_opt_set_sample_fmt(context.get(), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    if (swr_init(context.get()) < 0) {
        throw std::runtime_error("swr_init fail");
    }

    if (av_samples_alloc_array_and_samples(&inBuf, nullptr, inChannelNum,
                                           inSamples, AV_SAMPLE_FMT_S16, 0) < 0) {
        throw std::runtime_error("alloc in buf fail");
    }
    maxOutSamples = static_cast<int>(av_rescale_rnd(inSamples, outSampleRate, inSampleRate,
                                                    AV_ROUND_UP));
    if (av_samples_alloc_array_and_samples(&outBuf, &outLineSize, outChannelNum, maxOutSamples,
                                           AV_SAMPLE_FMT_S16, 0) < 0) {
        throw std::runtime_error("alloc out buf fail");
    }
}

AudioResampler::~AudioResampler() {
    if (inBuf) {
        av_freep(&inBuf[0]);
    }
    av_freep(&inBuf);
    if (outBuf) {
        av_freep(&outBuf[0]);
    }
    av_freep(outBuf);
}

int AudioResampler::resample(const void* inData, int inLen, void* outData) {
    int64_t delay = swr_get_delay(context.get(), inSampleRate);
    int outSamples = static_cast<int>(
            av_rescale_rnd(delay + inSamples, outSampleRate, inSampleRate, AV_ROUND_UP)
    );
    if (outSamples > maxOutSamples) {
        maxOutSamples = outSamples;
        av_freep(&outBuf[0]);
        if (av_samples_alloc_array_and_samples(&outBuf, &outLineSize, outChannelNum, maxOutSamples,
                                               AV_SAMPLE_FMT_S16, 0) < 0) {
            throw std::runtime_error("alloc out buf fail");
        }
    }

    memcpy(inBuf[0], inData, static_cast<size_t>(inLen));
    int ret = swr_convert(context.get(), outBuf, outSamples, (const uint8_t**) inBuf, inSamples);
    if (ret < 0) {
        return -2;
    }
    int outSize = av_samples_get_buffer_size(&outLineSize, outChannelNum, ret,
                                             AV_SAMPLE_FMT_S16, 1);
    memcpy(outData, outBuf[0], static_cast<size_t>(outSize));
    return outSize;
}

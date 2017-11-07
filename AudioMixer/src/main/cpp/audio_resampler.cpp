//
// Created by Piasy on 04/11/2017.
//

#include <rtc_base/checks.h>
#include <rtc_base/logging.h>

#include "audio_resampler.h"

AudioResampler::AudioResampler(int bytesPerSample, int inChannelNum, int inSampleRate,
                               int inSamples, int outChannelNum, int outSampleRate)
        : context(swr_alloc()),
          inBuf(nullptr),
          outBuf(nullptr),
          fmt(bytesPerSample == 1 ? AV_SAMPLE_FMT_U8 : AV_SAMPLE_FMT_S16),
          inSampleRate(inSampleRate),
          inChannelNum(inChannelNum),
          inSamples(inSamples),
          outSampleRate(outSampleRate),
          outChannelNum(outChannelNum) {
    RTC_CHECK(context.get());

    int64_t inChannelLayout = (inChannelNum == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    int64_t outChannelLayout = (outChannelNum == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;

    av_opt_set_int(context.get(), "in_channel_layout", inChannelLayout, 0);
    av_opt_set_int(context.get(), "in_sample_rate", inSampleRate, 0);
    av_opt_set_sample_fmt(context.get(), "in_sample_fmt", fmt, 0);

    av_opt_set_int(context.get(), "out_channel_layout", outChannelLayout, 0);
    av_opt_set_int(context.get(), "out_sample_rate", outSampleRate, 0);
    av_opt_set_sample_fmt(context.get(), "out_sample_fmt", fmt, 0);

    int error = swr_init(context.get());
    RTC_CHECK(error >= 0) << av_err2str(error);

    error = av_samples_alloc_array_and_samples(&inBuf, nullptr, inChannelNum, inSamples, fmt, 0);
    RTC_CHECK(error >= 0) << av_err2str(error);

    outSamples = static_cast<int>(av_rescale_rnd(inSamples, outSampleRate, inSampleRate,
                                                    AV_ROUND_UP));

    error = av_samples_alloc_array_and_samples(&outBuf, &outLineSize, outChannelNum, outSamples,
                                               fmt, 0);
    RTC_CHECK(error >= 0) << av_err2str(error);
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

uint8_t* AudioResampler::getInputBuffer() {
    return inBuf ? inBuf[0] : nullptr;
}

int AudioResampler::resample(const void* inData, int inLen, void* outData) {
    int64_t delay = swr_get_delay(context.get(), inSampleRate);
    if (inData != inBuf[0]) {
        memcpy(inBuf[0], inData, static_cast<size_t>(inLen));
    }
    int ret = swr_convert(context.get(), outBuf, outSamples, (const uint8_t**) inBuf, inSamples);
    LOG(LS_INFO) << outSamples << " " << ret;
    if (ret < 0) {
        return -2;
    }
    int outSize = av_samples_get_buffer_size(&outLineSize, outChannelNum, ret, fmt, 1);
    memcpy(outData, outBuf[0], static_cast<size_t>(outSize));
    return outSize;
}

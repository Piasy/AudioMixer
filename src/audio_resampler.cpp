//
// Created by Piasy on 04/11/2017.
//

#include <rtc_base/checks.h>

#include "audio_resampler.h"

AudioResampler::AudioResampler(AVSampleFormat input_format, int input_sample_rate,
                               int input_channel_num, int input_samples,
                               AVSampleFormat output_format, int output_sample_rate,
                               int output_channel_num)
        : context_(swr_alloc()),
          input_buffer_(nullptr),
          output_buffer_(nullptr),
          input_format_(input_format),
          input_sample_rate_(input_sample_rate),
          input_channel_num_(input_channel_num),
          input_samples_(input_samples),
          output_format_(output_format),
          output_sample_rate_(output_sample_rate),
          output_channel_num_(output_channel_num) {
    RTC_CHECK(context_.get());

    int64_t input_channel_layout = (input_channel_num_ == 1) ? AV_CH_LAYOUT_MONO
                                                             : AV_CH_LAYOUT_STEREO;
    int64_t output_channel_layout = (output_channel_num_ == 1) ? AV_CH_LAYOUT_MONO
                                                               : AV_CH_LAYOUT_STEREO;

    av_opt_set_int(context_.get(), "in_channel_layout", input_channel_layout, 0);
    av_opt_set_int(context_.get(), "in_sample_rate", input_sample_rate_, 0);
    av_opt_set_sample_fmt(context_.get(), "in_sample_fmt", input_format_, 0);

    av_opt_set_int(context_.get(), "out_channel_layout", output_channel_layout, 0);
    av_opt_set_int(context_.get(), "out_sample_rate", output_sample_rate_, 0);
    av_opt_set_sample_fmt(context_.get(), "out_sample_fmt", output_format_, 0);

    int error = swr_init(context_.get());
    RTC_CHECK(error >= 0) << av_err2str(error);

    error = av_samples_alloc_array_and_samples(reinterpret_cast<uint8_t***>(&input_buffer_),
                                               nullptr, input_channel_num_, input_samples_,
                                               input_format_, 0);
    RTC_CHECK(error >= 0) << av_err2str(error);

    output_samples_ = static_cast<int>(av_rescale_rnd(input_samples_, output_sample_rate_,
                                                      input_sample_rate_, AV_ROUND_UP));

    error = av_samples_alloc_array_and_samples(reinterpret_cast<uint8_t***>(&output_buffer_),
                                               nullptr, output_channel_num_, output_samples_,
                                               output_format_, 0);
    RTC_CHECK(error >= 0) << av_err2str(error);
}

AudioResampler::~AudioResampler() {
    if (input_buffer_) {
        av_freep(&input_buffer_[0]);
    }
    av_freep(&input_buffer_);
    if (output_buffer_) {
        av_freep(&output_buffer_[0]);
    }
    av_freep(output_buffer_);
}

void** AudioResampler::input_buffer() {
    return input_buffer_;
}

int AudioResampler::Resample(void** input_buffer, int input_size, void* output_buffer) {
    int64_t delay = swr_get_delay(context_.get(), input_sample_rate_);
    if (input_buffer[0] != input_buffer_[0]) {
        memcpy(input_buffer_[0], input_buffer[0], static_cast<size_t>(input_size));
    }
    int output_samples = swr_convert(context_.get(), reinterpret_cast<uint8_t**>(output_buffer_),
                                     output_samples_, (const uint8_t**) input_buffer_,
                                     input_samples_);
    if (output_samples < 0) {
        return output_samples;
    }
    int output_size = av_samples_get_buffer_size(nullptr, output_channel_num_, output_samples,
                                                 output_format_, 0);
    memcpy(output_buffer, output_buffer_[0], static_cast<size_t>(output_size));
    return output_size;
}

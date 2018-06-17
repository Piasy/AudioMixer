//
// Created by Piasy on 04/11/2017.
//

#include <rtc_base/checks.h>

#include "audio_resampler.h"

namespace audio_mixer {

AudioResampler::AudioResampler(
        AVSampleFormat input_format, int32_t input_sample_rate, int32_t input_channel_num,
        AVSampleFormat output_format, int32_t output_sample_rate, int32_t output_channel_num
)
        : context_(swr_alloc()),
          input_format_(input_format),
          input_sample_rate_(input_sample_rate),
          input_channel_num_(input_channel_num),
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

    int32_t error = swr_init(context_.get());
    //RTC_CHECK(error >= 0) << av_err2str(error);
}

int32_t AudioResampler::Resample(void** input_buffer, int32_t input_size, void** output_buffer) {
    int32_t input_samples = input_size / input_channel_num_
                            / av_get_bytes_per_sample(input_format_);
    int32_t output_samples = static_cast<int>(
            av_rescale_rnd(input_samples, output_sample_rate_, input_sample_rate_, AV_ROUND_UP)
    );

    int32_t real_output_samples = swr_convert(
            context_.get(), reinterpret_cast<uint8_t**>(output_buffer), output_samples,
            (const uint8_t**) input_buffer, input_samples
    );

    if (real_output_samples < 0) {
        return real_output_samples;
    }

    return av_samples_get_buffer_size(nullptr, output_channel_num_, real_output_samples,
                                      output_format_, 0);
}

}

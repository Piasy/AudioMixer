//
// Created by Piasy on 04/11/2017.
//

#pragma once

#include <memory>

#include "avx_helper.h"

namespace audio_mixer {

class AudioResampler {
public:
    AudioResampler(
            AVSampleFormat input_format, int32_t input_sample_rate, int32_t input_channel_num,
            AVSampleFormat output_format, int32_t output_sample_rate, int32_t output_channel_num
    );

    ~AudioResampler() {}

    int32_t Resample(void** input_buffer, int32_t input_size, void** output_buffer);

private:

    std::unique_ptr<SwrContext, SwrContextDeleter> context_;
    AVSampleFormat input_format_;
    int32_t input_sample_rate_;
    int32_t input_channel_num_;
    AVSampleFormat output_format_;
    int32_t output_sample_rate_;
    int32_t output_channel_num_;
};

}

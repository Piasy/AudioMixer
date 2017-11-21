//
// Created by Piasy on 04/11/2017.
//

#ifndef AUDIOMIXER_AUDIO_SAMPLER_H
#define AUDIOMIXER_AUDIO_SAMPLER_H


#include <memory>

#include "avx_helper.h"

class AudioResampler {
public:
    AudioResampler(AVSampleFormat input_format, int input_sample_rate, int input_channel_num,
                   int input_samples, AVSampleFormat output_format, int output_sample_rate,
                   int output_channel_num);

    ~AudioResampler();

    void** input_buffer();

    int Resample(void** input_buffer, int input_size, void* output_buffer);

private:

    std::unique_ptr<SwrContext, SwrContextDeleter> context_;
    void** input_buffer_;
    void** output_buffer_;
    AVSampleFormat input_format_;
    int input_sample_rate_;
    int input_channel_num_;
    int input_samples_;
    AVSampleFormat output_format_;
    int output_sample_rate_;
    int output_channel_num_;
    int output_samples_;
};


#endif //AUDIOMIXER_AUDIO_SAMPLER_H

//
// Created by Piasy on 29/10/2017.
//

#pragma once

#include <vector>

#include <api/audio/audio_mixer.h>
#include <rtc_base/scoped_ref_ptr.h>

#include "mixer_config.hpp"
#include "audio_record_source.h"

namespace audio_mixer {

class AudioMixer {
public:
    AudioMixer(const MixerConfig& config);

    ~AudioMixer();

    int32_t Mix(void* output_buffer);

    int32_t AddRecordedDataAndMix(const void* data, int32_t size, void* output_buffer);

private:
    rtc::scoped_refptr<webrtc::AudioMixer> mixer_;
    std::vector<std::unique_ptr<webrtc::AudioMixer::Source>> sources_;
    std::unique_ptr<AudioRecordSource> record_source_;
    std::unique_ptr<webrtc::AudioFrame> mixed_frame_;
    int32_t output_sample_rate_;
    int32_t output_channel_num_;
    int32_t output_samples_;
};

}

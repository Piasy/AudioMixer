//
// Created by Piasy on 29/10/2017.
//

#pragma once

#include <map>

#include <api/audio/audio_mixer.h>
#include <rtc_base/scoped_ref_ptr.h>

#include "audio_mixer_api.hpp"
#include "mixer_config.hpp"
#include "audio_source.h"
#include "audio_record_source.h"

namespace audio_mixer {

class AudioMixer : public AudioMixerApi {
public:
    AudioMixer(const MixerConfig& config);

    ~AudioMixer();

    void UpdateVolume(int32_t ssrc, float volume) override;

    bool AddSource(const MixerSource& source) override;

    bool RemoveSource(int32_t ssrc) override;

    int32_t Mix(void* output_buffer);

    void AddRecordedData(int32_t ssrc, const void* data, int32_t size);

private:
    std::shared_ptr<AudioSource> DoAddSource(const MixerSource& source);

    rtc::scoped_refptr<webrtc::AudioMixer> mixer_;
    std::map<int32_t, std::shared_ptr<AudioSource>> sources_;
    std::unique_ptr<webrtc::AudioFrame> mixed_frame_;
    int32_t output_sample_rate_;
    int32_t output_channel_num_;

    int32_t frame_duration_ms_;
    int32_t report_output_samples_;
    int32_t real_output_samples_;
};

}

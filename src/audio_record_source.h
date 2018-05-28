//
// Created by Piasy on 2018/5/28.
//

#pragma once

#include <rtc_base/buffer.h>

#include "audio_source.h"

namespace audio_mixer {

class AudioRecordSource : public AudioSource {
public:
    AudioRecordSource(int32_t ssrc, int32_t sample_rate, int32_t channel_num, float volume);

    ~AudioRecordSource();

    void OnAudioRecorded(const void* data, int32_t size);

    AudioFrameInfo
    GetAudioFrameWithInfo(int32_t sample_rate_hz, webrtc::AudioFrame* audio_frame) override;

    int32_t Ssrc() const override;

    int32_t PreferredSampleRate() const override;

private:
    int32_t ssrc_;

    int32_t sample_rate_;
    int32_t channel_num_;

    int32_t samples_per_channel_10ms_;
    int32_t buffer_num_elements_10ms_;

    rtc::BufferT<int16_t> buffer_;
};

}

//
// Created by Piasy on 29/10/2017.
//

#pragma once

#include <string>

#include "audio_source.h"
#include "audio_file_decoder.h"
#include "audio_resampler.h"

namespace audio_mixer {

class AudioFileSource : public AudioSource {
public:
    AudioFileSource(int32_t ssrc, const std::string& filepath, int32_t output_sample_rate,
                    int32_t output_channel_num, int32_t frame_duration_ms, float volume);

    ~AudioFileSource() override;

    AudioFrameInfo
    GetAudioFrameWithInfo(int32_t sample_rate_hz, webrtc::AudioFrame* audio_frame) override;

    int32_t Ssrc() const override;

    int32_t PreferredSampleRate() const override;

    int32_t input_sample_rate();

    int32_t input_channel_num();

    /**
     * @return > 0 for successfully read size
     *         AVERROR_EOF for end of file
     *         other value <= 0 for error
     */
    int32_t Read(void** buffer);

private:
    int32_t ssrc_;

    int32_t input_sample_rate_;
    int32_t input_channel_num_;
    AVSampleFormat input_format_;
    int32_t input_samples_;

    int32_t output_sample_rate_;
    int32_t output_channel_num_;

    int32_t frame_duration_ms_;
    int32_t report_output_samples_;
    int32_t real_output_samples_;

    void** input_buffer_;

    std::unique_ptr<AudioFileDecoder> decoder_;
    std::unique_ptr<AudioResampler> resampler_;
};

}

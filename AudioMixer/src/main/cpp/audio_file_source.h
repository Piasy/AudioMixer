//
// Created by Piasy on 29/10/2017.
//

#ifndef AUDIOMIXER_AUDIO_FILE_SOURCE_H
#define AUDIOMIXER_AUDIO_FILE_SOURCE_H

#include <string>

#include <api/audio/audio_mixer.h>

#include "audio_file_decoder.h"
#include "audio_resampler.h"

class AudioFileSource : public webrtc::AudioMixer::Source {
public:
    AudioFileSource(int ssrc, std::string& filepath, int output_sample_rate,
                    int output_channel_num);

    ~AudioFileSource() override;

    AudioFrameInfo
    GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) override;

    int Ssrc() const override;

    int PreferredSampleRate() const override;

    int input_sample_rate();

    int input_channel_num();

    /**
     * @return > 0 for successfully read size
     *         AVERROR_EOF for end of file
     *         other value <= 0 for error
     */
    int Read(void* buffer, int output_samples);

private:
    int ssrc;
    int sample_size_;
    int input_sample_rate_;
    int input_channel_num_;
    int input_samples_;
    int output_sample_rate_;
    int output_channel_num_;
    int output_samples_;

    std::unique_ptr<AudioFileDecoder> decoder_;
    std::unique_ptr<AudioResampler> resampler_;
};


#endif //AUDIOMIXER_AUDIO_FILE_SOURCE_H

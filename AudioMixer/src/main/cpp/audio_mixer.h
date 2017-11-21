//
// Created by Piasy on 29/10/2017.
//

#ifndef AUDIOMIXER_AUDIO_MIXER_H
#define AUDIOMIXER_AUDIO_MIXER_H

#include <vector>

#include <api/audio/audio_mixer.h>
#include <rtc_base/scoped_ref_ptr.h>

#include "audio_file_source.h"

class AudioMixer {
public:
    AudioMixer();

    ~AudioMixer();

    int Mix(void* buffer);

private:
    rtc::scoped_refptr<webrtc::AudioMixer> mixer_;
    std::vector<std::unique_ptr<AudioFileSource>> sources_;
    std::unique_ptr<webrtc::AudioFrame> mixed_frame_;
    size_t sample_size_;
    size_t output_sample_rate_;
    size_t output_channel_num_;
    size_t output_samples_;
};


#endif //AUDIOMIXER_AUDIO_MIXER_H

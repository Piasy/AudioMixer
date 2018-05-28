//
// Created by Piasy on 2018/5/28.
//

#pragma once

#include <api/audio/audio_mixer.h>

namespace audio_mixer {

class AudioSource : public webrtc::AudioMixer::Source {
public:
    AudioSource(float volume);

    void ApplyVolume(webrtc::AudioFrame* frame);

    void UpdateVolume(float volume);

private:
    float volume_;
};

}

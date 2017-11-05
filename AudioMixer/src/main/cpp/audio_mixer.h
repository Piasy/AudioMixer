//
// Created by Piasy on 29/10/2017.
//

#ifndef AUDIOMIXER_AUDIO_MIXER_H
#define AUDIOMIXER_AUDIO_MIXER_H

#include <vector>

#include <api/audio/audio_mixer.h>
#include <rtc_base/scoped_ref_ptr.h>

#include "file_audio_source.h"

class AudioMixer {
public:
    AudioMixer();

    ~AudioMixer();

    int mix(void* buf);

private:
    rtc::scoped_refptr<webrtc::AudioMixer> mixer;
    std::vector<std::unique_ptr<FileAudioSource>> sources;
    std::unique_ptr<webrtc::AudioFrame> mixFrame;
};


#endif //AUDIOMIXER_AUDIO_MIXER_H

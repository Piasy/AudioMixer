//
// Created by Piasy on 29/10/2017.
//

#ifndef HACKWEBRTC_AUDIO_MIXER_H
#define HACKWEBRTC_AUDIO_MIXER_H

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
    FileAudioSource* src1;
    FileAudioSource* src2;
    FileAudioSource* src3;
    FileAudioSource* src4;
    webrtc::AudioFrame* mixFrame;
};


#endif //HACKWEBRTC_AUDIO_MIXER_H

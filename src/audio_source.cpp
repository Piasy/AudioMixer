//
// Created by Piasy on 2018/5/28.
//

#include <audio/utility/audio_frame_operations.h>

#include "audio_source.h"

audio_mixer::AudioSource::AudioSource(float volume) : volume_(volume) {
}

void audio_mixer::AudioSource::ApplyVolume(webrtc::AudioFrame* frame) {
    if (volume_ < 0.99f || volume_ > 1.01f) {
        webrtc::AudioFrameOperations::ScaleWithSat(volume_, frame);
    }
}

void audio_mixer::AudioSource::UpdateVolume(float volume) {
    volume_ = volume;
}

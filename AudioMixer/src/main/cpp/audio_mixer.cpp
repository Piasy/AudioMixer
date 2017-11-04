//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_mixer.h"

AudioMixer::AudioMixer() : mixer(webrtc::AudioMixerImpl::Create()) {
    src1 = new FileAudioSource("/sdcard/1234.wav", 1234);
    src2 = new FileAudioSource("/sdcard/morning.wav", 1235);
    src3 = new FileAudioSource("/sdcard/lion.wav", 1236);
    src4 = new FileAudioSource("/sdcard/iamyou.wav", 1237);

    mixer->AddSource(src1);
    mixer->AddSource(src2);
    mixer->AddSource(src3);
    mixer->AddSource(src4);

    mixFrame = new webrtc::AudioFrame();
    mixFrame->sample_rate_hz_ = 48000;
    mixFrame->num_channels_ = 2;
    mixFrame->samples_per_channel_ = 480;
    mixFrame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    mixFrame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
}

AudioMixer::~AudioMixer() {
    mixer->RemoveSource(src1);
    mixer->RemoveSource(src2);
    mixer->RemoveSource(src3);
    mixer->RemoveSource(src4);
    delete src1;
    delete src2;
    delete src3;
    delete src4;
    delete mixFrame;
}

int AudioMixer::mix(void* buf) {
    mixer->Mix(2, mixFrame);
    int size = 48000 / 100 * 2 * 2;
    memcpy(buf, reinterpret_cast<const void*>(mixFrame->data()), static_cast<size_t>(size));
    return size;
}

//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_mixer.h"

AudioMixer::AudioMixer() : mixer(webrtc::AudioMixerImpl::Create()),
                           mixFrame(std::make_unique<webrtc::AudioFrame>()) {
    sources.push_back(std::make_unique<FileAudioSource>("/sdcard/wav/1234.raw", 1234));
    sources.push_back(std::make_unique<FileAudioSource>("/sdcard/wav/1235.raw", 1235));
    sources.push_back(std::make_unique<FileAudioSource>("/sdcard/wav/1236.raw", 1236));
    sources.push_back(std::make_unique<FileAudioSource>("/sdcard/wav/morning.raw", 1237));
    sources.push_back(std::make_unique<FileAudioSource>("/sdcard/wav/lion.raw", 1238));
    sources.push_back(std::make_unique<FileAudioSource>("/sdcard/wav/iamyou.raw", 1239));

    for (auto& source : sources) {
        mixer->AddSource(source.get());
    }

    mixFrame->sample_rate_hz_ = 48000;
    mixFrame->num_channels_ = 2;
    mixFrame->samples_per_channel_ = 480;
    mixFrame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    mixFrame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
}

AudioMixer::~AudioMixer() {
    for (auto& source : sources) {
        mixer->RemoveSource(source.get());
    }

    sources.clear();
}

int AudioMixer::mix(void* buf) {
    mixer->Mix(2, mixFrame.get());
    int size = 48000 / 100 * 2 * 2;
    memcpy(buf, reinterpret_cast<const void*>(mixFrame->data()), static_cast<size_t>(size));
    return size;
}

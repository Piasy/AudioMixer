//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_mixer.h"

AudioMixer::AudioMixer() : mixer_(webrtc::AudioMixerImpl::Create()),
                           mixed_frame_(std::make_unique<webrtc::AudioFrame>()),
                           sample_size_(2),
                           output_sample_rate_(48000),
                           output_channel_num_(1),
                           output_samples_(output_sample_rate_ /
                                         (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs)) {
    std::string f1("/sdcard/mp3/morning.mp3");
    sources_.push_back(std::make_unique<AudioFileSource>(
            1237, f1, output_sample_rate_, output_channel_num_
    ));
//    std::string f2("/sdcard/mp3/lion.mp3");
//    sources_.push_back(std::make_unique<AudioFileSource>(
//            1238, f2, output_sample_rate_, output_channel_num_
//    ));
//    std::string f3("/sdcard/mp3/iamyou.mp3");
//    sources_.push_back(std::make_unique<AudioFileSource>(
//            1239, f3, output_sample_rate_, output_channel_num_
//    ));

    for (auto& source : sources_) {
        mixer_->AddSource(source.get());
    }

    mixed_frame_->sample_rate_hz_ = output_sample_rate_;
    mixed_frame_->num_channels_ = output_channel_num_;
    mixed_frame_->samples_per_channel_ = output_samples_;
    mixed_frame_->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    mixed_frame_->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
}

AudioMixer::~AudioMixer() {
    for (auto& source : sources_) {
        mixer_->RemoveSource(source.get());
    }

    sources_.clear();
}

int AudioMixer::Mix(void* buffer) {
    mixer_->Mix(output_channel_num_, mixed_frame_.get());
    size_t size = output_samples_ * output_channel_num_ * sample_size_;
    memcpy(buffer, reinterpret_cast<const void*>(mixed_frame_->data()), size);
    return size;
}

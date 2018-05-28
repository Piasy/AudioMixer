//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_mixer_global.h"
#include "audio_mixer.h"
#include "audio_file_source.h"

namespace audio_mixer {

AudioMixer::AudioMixer(const MixerConfig& config)
        : mixer_(webrtc::AudioMixerImpl::Create()),
          mixed_frame_(std::make_unique<webrtc::AudioFrame>()),
          output_sample_rate_(config.output_sample_rate),
          output_channel_num_(config.output_channel_num),
          output_samples_(output_sample_rate_ / (1000 / MixerConfig::MS_PER_BUF)) {
    for (auto& f : config.input_files) {
        sources_.push_back(std::make_unique<AudioFileSource>(
                f, output_sample_rate_, output_channel_num_, MixerConfig::MS_PER_BUF
        ));
    }
    record_source_ = std::make_unique<AudioRecordSource>(output_sample_rate_,
                                                         output_channel_num_);

    for (auto& source : sources_) {
        mixer_->AddSource(source.get());
    }
    mixer_->AddSource(record_source_.get());

    mixed_frame_->sample_rate_hz_ = output_sample_rate_;
    mixed_frame_->num_channels_ = static_cast<size_t>(output_channel_num_);
    mixed_frame_->samples_per_channel_ = static_cast<size_t>(output_samples_);
    mixed_frame_->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    mixed_frame_->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
}

AudioMixer::~AudioMixer() {
    for (auto& source : sources_) {
        mixer_->RemoveSource(source.get());
    }
    mixer_->RemoveSource(record_source_.get());

    sources_.clear();
}

int32_t AudioMixer::Mix(void* output_buffer) {
    mixer_->Mix(static_cast<size_t>(output_channel_num_), mixed_frame_.get());

    int32_t size = av_samples_get_buffer_size(nullptr, output_channel_num_, output_samples_,
                                              kOutputSampleFormat, 1);
    memcpy(output_buffer, reinterpret_cast<const void*>(mixed_frame_->data()),
           static_cast<size_t>(size));
    return size;
}

int32_t AudioMixer::AddRecordedDataAndMix(const void* data, int32_t size, void* output_buffer) {
    record_source_->OnAudioRecorded(data, size);

    return Mix(output_buffer);
}

}

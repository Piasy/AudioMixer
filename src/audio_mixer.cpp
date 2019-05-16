//
// Created by Piasy on 29/10/2017.
//

#include <rtc_base/checks.h>
#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_mixer.h"
#include "audio_mixer_global.h"
#include "audio_file_source.h"
#include "mixer_source.hpp"

namespace audio_mixer {

std::shared_ptr<AudioMixerApi> AudioMixerApi::Create(const MixerConfig& config) {
    return std::make_shared<AudioMixer>(config);
}

AudioMixer::AudioMixer(const MixerConfig& config)
        : mixer_(webrtc::AudioMixerImpl::Create()),
          mixed_frame_(std::make_unique<webrtc::AudioFrame>()),
          output_sample_rate_(config.output_sample_rate),
          output_channel_num_(config.output_channel_num) {
    RTC_CHECK(config.frame_duration_ms <= webrtc::AudioMixerImpl::kFrameDurationInMs)
    << "frame duration too long";

    frame_duration_ms_ = config.frame_duration_ms;
    report_output_samples_ =
            output_sample_rate_ / (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs);
    real_output_samples_ = output_sample_rate_ / (1000 / frame_duration_ms_);

    for (auto& source : config.sources) {
        DoAddSource(source);
    }
    for (const auto& item : sources_) {
        mixer_->AddSource(item.second.get());
    }

    mixed_frame_->UpdateFrame(0, nullptr, static_cast<size_t>(report_output_samples_),
                              output_sample_rate_,
                              webrtc::AudioFrame::SpeechType::kUndefined,
                              webrtc::AudioFrame::VADActivity::kVadUnknown,
                              static_cast<size_t>(output_channel_num_));
}

AudioMixer::~AudioMixer() {
    for (const auto& item : sources_) {
        mixer_->RemoveSource(item.second.get());
    }

    sources_.clear();
}

void AudioMixer::UpdateVolume(int32_t ssrc, float volume) {
    auto source = sources_.find(ssrc);
    if (source != sources_.end()) {
        source->second->UpdateVolume(volume);
    }
}

bool AudioMixer::AddSource(const MixerSource& source) {
    std::shared_ptr<AudioSource> audio_source = DoAddSource(source);
    mixer_->AddSource(audio_source.get());

    return true;
}

bool AudioMixer::RemoveSource(int32_t ssrc) {
    auto source = sources_.find(ssrc);
    if (source != sources_.end()) {
        mixer_->RemoveSource(source->second.get());
        sources_.erase(source);

        return true;
    }

    return false;
}

int32_t AudioMixer::Mix(void* output_buffer) {
    mixer_->Mix(static_cast<size_t>(output_channel_num_), mixed_frame_.get());

    int32_t size = av_samples_get_buffer_size(nullptr, output_channel_num_, real_output_samples_,
                                              kOutputSampleFormat, 1);
    memcpy(output_buffer, reinterpret_cast<const void*>(mixed_frame_->data()),
           static_cast<size_t>(size));
    return size;
}

void AudioMixer::AddRecordedData(int32_t ssrc, const void* data, int32_t size) {
    auto source = sources_.find(ssrc);
    if (source != sources_.end()) {
        reinterpret_cast<AudioRecordSource*>(source->second.get())->OnAudioRecorded(data, size);
    }
}

std::shared_ptr<AudioSource> AudioMixer::DoAddSource(const MixerSource& source) {
    if (source.type == MixerSource::TYPE_RECORD) {
        RTC_CHECK(source.sample_rate == output_sample_rate_)
        << "record source must have the same sample rate as output";
        RTC_CHECK(source.channel_num == output_channel_num_)
        << "record source must have the same channels as output";

        std::shared_ptr<AudioRecordSource> record_source = std::make_shared<AudioRecordSource>(
                source.ssrc, output_sample_rate_, output_channel_num_, frame_duration_ms_,
                source.volume
        );
        sources_.insert(std::pair<int32_t, std::shared_ptr<AudioSource>>(
                source.ssrc, record_source
        ));

        return record_source;
    } else {
        std::shared_ptr<AudioSource> file_source = std::make_shared<AudioFileSource>(
                source.ssrc, source.path, output_sample_rate_, output_channel_num_,
                frame_duration_ms_, source.volume
        );
        sources_.emplace(std::make_pair(source.ssrc, file_source));

        return file_source;
    }
}

}

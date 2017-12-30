//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_file_source.h"


AudioFileSource::AudioFileSource(int ssrc, std::string& filepath, int output_sample_rate,
                                 int output_channel_num)
        : ssrc(ssrc),
          output_sample_rate_(output_sample_rate),
          output_channel_num_(output_channel_num),
          output_samples_(
                  output_sample_rate / (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs)) {
    decoder_.reset(new AudioFileDecoder(filepath));

    AVSampleFormat input_format = decoder_->sample_format();
    sample_size_ = av_get_bytes_per_sample(input_format);
    input_sample_rate_ = decoder_->sample_rate();
    input_samples_ = input_sample_rate_ / (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs);
    input_channel_num_ = decoder_->channel_num();

    resampler_.reset(new AudioResampler(input_format, input_sample_rate_, input_channel_num_,
                                        input_samples_, AV_SAMPLE_FMT_S16, output_sample_rate_,
                                        output_channel_num_));
}

AudioFileSource::~AudioFileSource() {
}

webrtc::AudioMixer::Source::AudioFrameInfo
AudioFileSource::GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) {
    if (sample_rate_hz != output_sample_rate_) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    if (Read(reinterpret_cast<void*>(audio_frame->mutable_data()), output_samples_) == -1) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    audio_frame->sample_rate_hz_ = output_sample_rate_;
    audio_frame->num_channels_ = static_cast<size_t>(output_channel_num_);
    audio_frame->samples_per_channel_ = static_cast<size_t>(output_samples_);
    audio_frame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    audio_frame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
    return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
}

int AudioFileSource::Ssrc() const {
    return ssrc;
}

int AudioFileSource::PreferredSampleRate() const {
    return output_sample_rate_;
}

int AudioFileSource::input_sample_rate() {
    return input_sample_rate_;
}

int AudioFileSource::input_channel_num() {
    return input_channel_num_;
}

int AudioFileSource::Read(void* buffer, int output_samples) {
    void** input_buffer = resampler_->input_buffer();
    int input_samples = static_cast<int>(av_rescale_rnd(output_samples, input_sample_rate_,
                                                        output_sample_rate_, AV_ROUND_UP));
    int consumed = decoder_->Consume(input_buffer, input_samples);
    if (consumed != input_samples * input_channel_num_ * sample_size_) {
        return AVERROR_EOF;
    }

    int resampled = resampler_->Resample(input_buffer, consumed, buffer);
    if (resampled <= 0) {
        return resampled;
    }

    return resampled;
}

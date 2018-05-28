//
// Created by Piasy on 29/10/2017.
//

#include <rtc_base/checks.h>

#include "audio_mixer_global.h"
#include "audio_file_source.h"

namespace audio_mixer {

AudioFileSource::AudioFileSource(const std::string& filepath, int32_t output_sample_rate,
                                 int32_t output_channel_num, int32_t msPerBuf)
        : ssrc_(g_ssrc++),
          output_sample_rate_(output_sample_rate),
          output_channel_num_(output_channel_num),
          output_samples_(output_sample_rate / (1000 / msPerBuf)),
          input_buffer_(nullptr) {
    decoder_.reset(new AudioFileDecoder(filepath));

    input_sample_rate_ = decoder_->sample_rate();
    input_channel_num_ = decoder_->channel_num();
    input_format_ = decoder_->sample_format();
    input_samples_ = input_sample_rate_ / (1000 / msPerBuf);

    int32_t error = av_samples_alloc_array_and_samples(
            reinterpret_cast<uint8_t***>(&input_buffer_), nullptr, input_channel_num_, 
            input_samples_, input_format_, 0
    );
    RTC_CHECK(error >= 0) << av_err2str(error);

    resampler_.reset(new AudioResampler(input_format_, input_sample_rate_, input_channel_num_,
                                        kOutputSampleFormat, output_sample_rate_,
                                        output_channel_num_));
}

AudioFileSource::~AudioFileSource() {
    if (input_buffer_) {
        av_freep(&input_buffer_[0]);
    }
    av_freep(&input_buffer_);
}

webrtc::AudioMixer::Source::AudioFrameInfo
AudioFileSource::GetAudioFrameWithInfo(int32_t sample_rate_hz, webrtc::AudioFrame* audio_frame) {
    if (sample_rate_hz != output_sample_rate_) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    int16_t* output_buffer = audio_frame->mutable_data();
    if (Read(reinterpret_cast<void**>(&output_buffer)) < 0) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    audio_frame->sample_rate_hz_ = output_sample_rate_;
    audio_frame->num_channels_ = static_cast<size_t>(output_channel_num_);
    audio_frame->samples_per_channel_ = static_cast<size_t>(output_samples_);
    audio_frame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    audio_frame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
    return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
}

int32_t AudioFileSource::Ssrc() const {
    return ssrc_;
}

int32_t AudioFileSource::PreferredSampleRate() const {
    return output_sample_rate_;
}

int32_t AudioFileSource::input_sample_rate() {
    return input_sample_rate_;
}

int32_t AudioFileSource::input_channel_num() {
    return input_channel_num_;
}

int32_t AudioFileSource::Read(void** buffer) {
    int32_t consumed = decoder_->Consume(input_buffer_, input_samples_);
    // don't align with 32 bytes
    if (consumed != av_samples_get_buffer_size(nullptr, input_channel_num_, input_samples_,
                                               input_format_, 1)) {
        return AVERROR_EOF;
    }

    return resampler_->Resample(input_buffer_, consumed, buffer);
}

}

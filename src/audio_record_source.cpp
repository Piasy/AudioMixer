//
// Created by Piasy on 2018/5/28.
//

#include "audio_record_source.h"
#include "audio_mixer_global.h"

audio_mixer::AudioRecordSource::AudioRecordSource(int32_t sample_rate, int32_t channel_num)
        : ssrc_(g_ssrc++),
          sample_rate_(sample_rate),
          channel_num_(channel_num),
          samples_per_channel_10ms_(sample_rate * 10 / 1000),
          buffer_num_elements_10ms_(channel_num * samples_per_channel_10ms_) {
    buffer_.Clear();
}

audio_mixer::AudioRecordSource::~AudioRecordSource() {
}

void audio_mixer::AudioRecordSource::OnAudioRecorded(const void* data, int32_t size) {
    buffer_.AppendData(static_cast<const int16_t*>(data),
                       static_cast<size_t>(size / sizeof(int16_t)));
}

webrtc::AudioMixer::Source::AudioFrameInfo
audio_mixer::AudioRecordSource::GetAudioFrameWithInfo(int32_t sample_rate_hz,
                                                      webrtc::AudioFrame* audio_frame) {
    if (sample_rate_hz != sample_rate_) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }
    if (buffer_.size() < buffer_num_elements_10ms_) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kMuted;
    }

    audio_frame->UpdateFrame(0, buffer_.data(),
                             static_cast<size_t>(samples_per_channel_10ms_),
                             sample_rate_,
                             webrtc::AudioFrame::SpeechType::kNormalSpeech,
                             webrtc::AudioFrame::VADActivity::kVadActive,
                             static_cast<size_t>(channel_num_));

    memmove(buffer_.data(), buffer_.data() + buffer_num_elements_10ms_,
            (buffer_.size() - buffer_num_elements_10ms_) * sizeof(int16_t));
    buffer_.SetSize(buffer_.size() - buffer_num_elements_10ms_);

    return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
}

int32_t audio_mixer::AudioRecordSource::Ssrc() const {
    return ssrc_;
}

int32_t audio_mixer::AudioRecordSource::PreferredSampleRate() const {
    return sample_rate_;
}

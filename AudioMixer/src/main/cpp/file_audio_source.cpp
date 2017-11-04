//
// Created by Piasy on 29/10/2017.
//

#include <rtc_base/logging.h>

#include "file_audio_source.h"


FileAudioSource::FileAudioSource(std::string filename, int ssrc)
        : pcm(new std::ifstream()), ssrc(ssrc) {
    pcm->open(filename, std::ios::in | std::ios::binary);
}

FileAudioSource::~FileAudioSource() {
    if (pcm->is_open()) {
        pcm->close();
    }
}

int FileAudioSource::Ssrc() const {
    return ssrc;
}

int FileAudioSource::PreferredSampleRate() const {
    return 48000;
}

webrtc::AudioMixer::Source::AudioFrameInfo
FileAudioSource::GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) {
    if (!pcm->is_open()) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }
    audio_frame->sample_rate_hz_ = 48000;
    audio_frame->num_channels_ = 2;
    audio_frame->samples_per_channel_ = 480;
    audio_frame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    audio_frame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
    pcm->read(reinterpret_cast<char*>(audio_frame->mutable_data()),
              audio_frame->samples_per_channel_ * audio_frame->num_channels_ * 2);
    return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
}

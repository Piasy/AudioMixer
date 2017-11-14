//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_file_source.h"


AudioFileSource::AudioFileSource(int ssrc, std::string filename, int bytesPerSample_,
                                 int inputSampleRate_, int inputChannelNum_, int outputSampleRate_,
                                 int outputChannelNum_)
        : ssrc(ssrc),
          pcm(std::make_unique<std::ifstream>()),
          bytesPerSample(static_cast<size_t>(bytesPerSample_)),
          inputSampleRate(static_cast<size_t>(inputSampleRate_)),
          inputChannelNum(static_cast<size_t>(inputChannelNum_)),
          inputSamples(inputSampleRate /
                       (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs)),
          outputSampleRate(static_cast<size_t>(outputSampleRate_)),
          outputChannelNum(static_cast<size_t>(outputChannelNum_)),
          outputSamples(outputSampleRate /
                        (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs)) {
    pcm->open(filename, std::ios::in | std::ios::binary);

    resampler.reset(new AudioResampler(bytesPerSample, inputChannelNum, inputSampleRate,
                                       inputSamples, outputChannelNum, outputSampleRate));
}

AudioFileSource::~AudioFileSource() {
    if (pcm->is_open()) {
        pcm->close();
    }
}

int AudioFileSource::Ssrc() const {
    return ssrc;
}

int AudioFileSource::PreferredSampleRate() const {
    return outputSampleRate;
}

webrtc::AudioMixer::Source::AudioFrameInfo
AudioFileSource::GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) {
    if (sample_rate_hz != outputSampleRate || !pcm->is_open()) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    void* inputBuffer = resampler->getInputBuffer();
    int inputSize = inputSamples * inputChannelNum * bytesPerSample;
    int read = pcm->read(reinterpret_cast<char*>(inputBuffer), inputSize).gcount();
    if (read != inputSize) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    void* outputBuffer = reinterpret_cast<void*>(audio_frame->mutable_data());
    if (resampler->resample(inputBuffer, inputSize, outputBuffer) <= 0) {
        return webrtc::AudioMixer::Source::AudioFrameInfo::kError;
    }

    audio_frame->sample_rate_hz_ = outputSampleRate;
    audio_frame->num_channels_ = outputChannelNum;
    audio_frame->samples_per_channel_ = outputSamples;
    audio_frame->speech_type_ = webrtc::AudioFrame::SpeechType::kNormalSpeech;
    audio_frame->vad_activity_ = webrtc::AudioFrame::VADActivity::kVadActive;
    return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
}

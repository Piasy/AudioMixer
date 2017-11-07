//
// Created by Piasy on 29/10/2017.
//

#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_mixer.h"

AudioMixer::AudioMixer() : mixer(webrtc::AudioMixerImpl::Create()),
                           mixFrame(std::make_unique<webrtc::AudioFrame>()),
                           bytesPerSample(2),
                           outputSampleRate(48000),
                           outputChannelNum(1),
                           outputSamples(outputSampleRate /
                                         (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs)) {
//    sources.push_back(std::make_unique<FileAudioSource>(
//            1234, "/sdcard/wav/1234.raw", bytesPerSample, 44100, 2, outputSampleRate,
//            outputChannelNum
//    ));
//    sources.push_back(std::make_unique<FileAudioSource>(
//            1235, "/sdcard/wav/1235.raw", bytesPerSample, 44100, 2, outputSampleRate,
//            outputChannelNum
//    ));
//    sources.push_back(std::make_unique<FileAudioSource>(
//            1236, "/sdcard/wav/1236.raw", bytesPerSample, 44100, 2, outputSampleRate,
//            outputChannelNum
//    ));
    sources.push_back(std::make_unique<FileAudioSource>(
            1237, "/sdcard/wav/morning.raw", bytesPerSample, 44100, 2, outputSampleRate,
            outputChannelNum
    ));
    sources.push_back(std::make_unique<FileAudioSource>(
            1238, "/sdcard/wav/lion.raw", bytesPerSample, 44100, 2, outputSampleRate,
            outputChannelNum
    ));
    sources.push_back(std::make_unique<FileAudioSource>(
            1239, "/sdcard/wav/iamyou.raw", bytesPerSample, 44100, 2, outputSampleRate,
            outputChannelNum
    ));

    for (auto& source : sources) {
        mixer->AddSource(source.get());
    }

    mixFrame->sample_rate_hz_ = outputSampleRate;
    mixFrame->num_channels_ = outputChannelNum;
    mixFrame->samples_per_channel_ = outputSamples;
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
    mixer->Mix(outputChannelNum, mixFrame.get());
    size_t size = outputSamples * outputChannelNum * bytesPerSample;
    memcpy(buf, reinterpret_cast<const void*>(mixFrame->data()), size);
    return size;
}

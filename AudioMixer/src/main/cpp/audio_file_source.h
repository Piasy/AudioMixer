//
// Created by Piasy on 29/10/2017.
//

#ifndef AUDIOMIXER_FILE_AUDIO_SOURCE_H
#define AUDIOMIXER_FILE_AUDIO_SOURCE_H

#include <fstream>
#include <string>

#include <api/audio/audio_mixer.h>

#include "audio_resampler.h"

class AudioFileSource : public webrtc::AudioMixer::Source {
public:
    AudioFileSource(int ssrc, std::string filename, int bytesPerSample, int inputSampleRate,
                    int inputChannelNum, int outputSampleRate, int outputChannelNum);

    ~AudioFileSource() override;

    AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz,
                                         webrtc::AudioFrame* audio_frame) override;

    int Ssrc() const override;

    int PreferredSampleRate() const override;

private:
    int ssrc;
    std::unique_ptr<std::ifstream> pcm;
    size_t bytesPerSample;
    size_t inputSampleRate;
    size_t inputChannelNum;
    size_t inputSamples;
    size_t outputSampleRate;
    size_t outputChannelNum;
    size_t outputSamples;
    std::unique_ptr<AudioResampler> resampler;
};


#endif //AUDIOMIXER_FILE_AUDIO_SOURCE_H

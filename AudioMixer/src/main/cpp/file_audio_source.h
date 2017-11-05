//
// Created by Piasy on 29/10/2017.
//

#ifndef AUDIOMIXER_FILE_AUDIO_SOURCE_H
#define AUDIOMIXER_FILE_AUDIO_SOURCE_H

#include <fstream>
#include <string>

#include <api/audio/audio_mixer.h>

class FileAudioSource : public webrtc::AudioMixer::Source {
public:
    FileAudioSource(std::string filename, int ssrc);

    ~FileAudioSource() override;

    AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz,
                                         webrtc::AudioFrame* audio_frame) override;

    int Ssrc() const override;

    int PreferredSampleRate() const override;

private:
    std::unique_ptr<std::ifstream> pcm;
    int ssrc;
};


#endif //AUDIOMIXER_FILE_AUDIO_SOURCE_H

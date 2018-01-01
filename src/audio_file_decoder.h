//
// Created by Piasy on 08/11/2017.
//

#pragma once

#include <string>
#include <vector>

#include "avx_helper.h"

namespace audio_mixer {

class AudioFileDecoder {
public:
    AudioFileDecoder(const std::string& filepath);

    ~AudioFileDecoder();

    AVSampleFormat sample_format();

    int32_t sample_rate();

    int32_t channel_num();

    int32_t Consume(void** buffer, int32_t samples);

private:
    void FillDecoder();

    void FillFifo();

    int32_t fifo_capacity_;
    int32_t stream_no_;

    std::unique_ptr<AVFormatContext, AVFormatContextDeleter> format_context_;
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> codec_context_;

    std::unique_ptr<AVPacket, AVPacketDeleter> packet_;
    bool packet_consumed_;
    std::unique_ptr<AVFrame, AVFrameDeleter> frame_;
    std::unique_ptr<AVAudioFifo, AVAudioFifoDeleter> fifo_;
};

}

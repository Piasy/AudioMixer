//
// Created by Piasy on 08/11/2017.
//

#ifndef AUDIOMIXER_AUDIO_FILE_DECODER_H
#define AUDIOMIXER_AUDIO_FILE_DECODER_H

#include <string>
#include <vector>

#include "avx_helper.h"

class AudioFileDecoder {
public:
    AudioFileDecoder(std::string& filepath);

    ~AudioFileDecoder();

    AVSampleFormat sample_format();

    int sample_rate();

    int channel_num();

    int Consume(void** buffer, int samples);

private:
    void FillDecoder();

    void FillFifo();

    int fifo_capacity_;
    int stream_no_;

    std::unique_ptr<AVFormatContext, AVFormatContextDeleter> format_context_;
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> codec_context_;

    std::unique_ptr<AVPacket, AVPacketDeleter> packet_;
    bool packet_consumed_;
    std::unique_ptr<AVFrame, AVFrameDeleter> frame_;
    std::unique_ptr<AVAudioFifo, AVAudioFifoDeleter> fifo_;
};


#endif //AUDIOMIXER_AUDIO_FILE_DECODER_H

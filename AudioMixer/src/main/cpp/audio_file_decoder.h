//
// Created by Piasy on 08/11/2017.
//

#ifndef AUDIOMIXER_AUDIO_DECODER_H
#define AUDIOMIXER_AUDIO_DECODER_H

#include <string>
#include <vector>

#include "avx_helper.h"

class AudioFileDecoder {
public:
    AudioFileDecoder(std::string& filename);

    ~AudioFileDecoder();

    int getSampleRate();

    int getChannelNum();

    int consume(void* buffer, int samples);

private:
    void fillDecoder();

    void fillFifo();

    int fifoCapacity;
    int streamNo;

    std::unique_ptr<AVFormatContext, AVFormatContextDeleter> avFormatContext;
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> avCodecContext;

    std::unique_ptr<AVPacket, AVPacketDeleter> packet;
    bool packetConsumed;
    std::unique_ptr<AVFrame, AVFrameDeleter> frame;
    std::unique_ptr<AVAudioFifo, AVAudioFifoDeleter> fifo;
};


#endif //AUDIOMIXER_AUDIO_DECODER_H

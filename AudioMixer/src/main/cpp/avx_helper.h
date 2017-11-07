//
// Created by Piasy on 08/11/2017.
//

#ifndef AUDIOMIXER_AVX_DELETER_H
#define AUDIOMIXER_AVX_DELETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif

struct SwrContextDeleter {
    void operator()(SwrContext* swrContext) {
        swr_free(&swrContext);
    }
};

struct AVFormatContextDeleter {
    void operator()(AVFormatContext* context) {
        if (context) {
            avformat_close_input(&context);
        }
    }
};

struct AVCodecContextDeleter {
    void operator()(AVCodecContext* context) {
        if (context) {
            avcodec_free_context(&context);
        }
    }
};

struct AVFrameDeleter {
    void operator()(AVFrame* frame) {
        if (frame) {
            av_frame_free(&frame);
        }
    }
};

struct AVPacketDeleter {
    void operator()(AVPacket* packet) {
        if (packet) {
            av_packet_free(&packet);
        }
    }
};

#endif //AUDIOMIXER_AVX_DELETER_H

//
// Created by Piasy on 08/11/2017.
//

#include <algorithm>

#include <rtc_base/checks.h>
#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_file_decoder.h"

AudioFileDecoder::AudioFileDecoder(std::string& filename) : packetConsumed(true) {
    frame.reset(av_frame_alloc());
    RTC_CHECK(frame.get()) << "av_frame_alloc fail";

    packet.reset(av_packet_alloc());
    RTC_CHECK(packet.get()) << "av_packet_alloc fail";
    av_init_packet(packet.get());

    {
        AVFormatContext* formatContext = nullptr;
        int error = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
        RTC_CHECK(error >= 0) << av_err2str(error);

        avFormatContext.reset(formatContext);
    }

    AVCodec* codec;
    int error = avformat_find_stream_info(avFormatContext.get(), nullptr);
    RTC_CHECK(error >= 0) << av_err2str(error);

    streamNo = av_find_best_stream(avFormatContext.get(), AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    RTC_CHECK(streamNo >= 0) << av_err2str(streamNo);

    if (!(codec = avcodec_find_decoder(avFormatContext->streams[streamNo]->codecpar->codec_id))) {
        RTC_CHECK(false) << "avcodec_find_decoder fail";
    }
    avCodecContext.reset(avcodec_alloc_context3(codec));
    RTC_CHECK(avCodecContext.get()) << "avcodec_alloc_context3 fail";
    error = avcodec_parameters_to_context(avCodecContext.get(),
                                          avFormatContext->streams[streamNo]->codecpar);
    RTC_CHECK(error >= 0) << av_err2str(error);

    error = avcodec_open2(avCodecContext.get(), codec, nullptr);
    RTC_CHECK(error >= 0) << av_err2str(error);

    fifoCapacity = 10 * avCodecContext->sample_rate
                   / (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs);
    fifo.reset(av_audio_fifo_alloc(avCodecContext->sample_fmt, avCodecContext->channels,
                                   fifoCapacity));
    RTC_CHECK(fifo.get()) << "av_audio_fifo_alloc fail";

    fillDecoder();
}

AudioFileDecoder::~AudioFileDecoder() {
}

int AudioFileDecoder::getSampleRate() {
    return avCodecContext->sample_rate;
}

int AudioFileDecoder::getChannelNum() {
    return avCodecContext->channels;
}

int AudioFileDecoder::consume(void* buffer, int samples) {
    fillDecoder();
    fillFifo();

    int targetSamples = std::min(av_audio_fifo_size(fifo.get()), samples);

    int actualSamples = av_audio_fifo_read(fifo.get(), &buffer, targetSamples);
    int size = actualSamples * 2 * avCodecContext->channels;

    return size;
}

void AudioFileDecoder::fillDecoder() {
    while (true) {
        if (packetConsumed) {
            if (av_read_frame(avFormatContext.get(), packet.get()) != 0) {
                break;
            }
            if (packet->stream_index != streamNo) {
                av_packet_unref(packet.get());
                continue;
            }
            packetConsumed = false;
        }
        int error = avcodec_send_packet(avCodecContext.get(), packet.get());
        if (error == 0) {
            av_packet_unref(packet.get());
            packetConsumed = true;
            continue;
        }
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            break;
        }
        RTC_CHECK(false) << av_err2str(error);
    }
}

void AudioFileDecoder::fillFifo() {
    while (av_audio_fifo_size(fifo.get()) < fifoCapacity
           && avcodec_receive_frame(avCodecContext.get(), frame.get()) == 0) {
        av_audio_fifo_write(fifo.get(), reinterpret_cast<void**>(frame->extended_data),
                            frame->nb_samples);
        av_frame_unref(frame.get());
    }
}

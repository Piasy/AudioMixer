//
// Created by Piasy on 08/11/2017.
//

#include <algorithm>

#include <rtc_base/checks.h>
#include <modules/audio_mixer/audio_mixer_impl.h>

#include "audio_file_decoder.h"

namespace audio_mixer {

AudioFileDecoder::AudioFileDecoder(const std::string& filepath) : packet_consumed_(true) {
    frame_.reset(av_frame_alloc());
    RTC_CHECK(frame_.get()) << "av_frame_alloc fail";

    packet_.reset(av_packet_alloc());
    RTC_CHECK(packet_.get()) << "av_packet_alloc fail";
    av_init_packet(packet_.get());

    {
        AVFormatContext* format_context = nullptr;
        int32_t error = avformat_open_input(&format_context, filepath.c_str(), nullptr, nullptr);
        RTC_CHECK(error >= 0) << av_err2str(error);

        format_context_.reset(format_context);
    }

    AVCodec* codec;
    int32_t error = avformat_find_stream_info(format_context_.get(), nullptr);
    RTC_CHECK(error >= 0) << av_err2str(error);

    stream_no_ = av_find_best_stream(format_context_.get(), AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    RTC_CHECK(stream_no_ >= 0) << av_err2str(stream_no_);

    if (!(codec = avcodec_find_decoder(format_context_->streams[stream_no_]->codecpar->codec_id))) {
        RTC_CHECK(false) << "avcodec_find_decoder fail";
    }
    codec_context_.reset(avcodec_alloc_context3(codec));
    RTC_CHECK(codec_context_.get()) << "avcodec_alloc_context3 fail";
    error = avcodec_parameters_to_context(codec_context_.get(),
                                          format_context_->streams[stream_no_]->codecpar);
    RTC_CHECK(error >= 0) << av_err2str(error);

    error = avcodec_open2(codec_context_.get(), codec, nullptr);
    RTC_CHECK(error >= 0) << av_err2str(error);

    fifo_capacity_ = 10 * codec_context_->sample_rate
                     / (1000 / webrtc::AudioMixerImpl::kFrameDurationInMs);
    fifo_.reset(av_audio_fifo_alloc(codec_context_->sample_fmt, codec_context_->channels,
                                    fifo_capacity_));
    RTC_CHECK(fifo_.get()) << "av_audio_fifo_alloc fail";

    FillDecoder();
}

AudioFileDecoder::~AudioFileDecoder() {
}

AVSampleFormat AudioFileDecoder::sample_format() {
    return codec_context_->sample_fmt;
}

int32_t AudioFileDecoder::sample_rate() {
    return codec_context_->sample_rate;
}

int32_t AudioFileDecoder::channel_num() {
    return codec_context_->channels;
}

int32_t AudioFileDecoder::Consume(void** buffer, int32_t samples) {
    FillDecoder();
    FillFifo();

    int32_t target_samples = std::min(av_audio_fifo_size(fifo_.get()), samples);
    int32_t actual_samples = av_audio_fifo_read(fifo_.get(), buffer, target_samples);

    return actual_samples * 2 * codec_context_->channels;
}

void AudioFileDecoder::FillDecoder() {
    while (true) {
        if (packet_consumed_) {
            if (av_read_frame(format_context_.get(), packet_.get()) != 0) {
                break;
            }
            if (packet_->stream_index != stream_no_) {
                av_packet_unref(packet_.get());
                continue;
            }
            packet_consumed_ = false;
        }
        int32_t error = avcodec_send_packet(codec_context_.get(), packet_.get());
        if (error == 0) {
            av_packet_unref(packet_.get());
            packet_consumed_ = true;
            continue;
        }
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            break;
        }
        RTC_CHECK(false) << av_err2str(error);
    }
}

void AudioFileDecoder::FillFifo() {
    while (av_audio_fifo_size(fifo_.get()) < fifo_capacity_
           && avcodec_receive_frame(codec_context_.get(), frame_.get()) == 0) {
        av_audio_fifo_write(fifo_.get(), reinterpret_cast<void**>(frame_->extended_data),
                            frame_->nb_samples);
        av_frame_unref(frame_.get());
    }
}

}

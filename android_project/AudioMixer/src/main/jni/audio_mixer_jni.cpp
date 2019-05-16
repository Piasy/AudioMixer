//
// Created by Piasy on 02/09/2017.
//

#include <string>
#include <jni.h>

#include <sdk/android/src/jni/jni_helpers.h>
#include <rtc_base/logging.h>

#include "audio_mixer_global.h"
#include "avx_helper.h"
#include "audio_resampler.h"
#include "audio_file_decoder.h"
#include "audio_file_source.h"
#include "audio_mixer.h"

#include "Marshal.hpp"
#include "NativeMixerConfig.hpp"

namespace audio_mixer {
extern "C" {

#define toJ(handle) webrtc::jni::jlongFromPointer((handle))
#define fromJ(type, handle) reinterpret_cast<type *>((handle))

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_globalInitializeFFmpeg(JNIEnv* env, jclass type) {
    av_register_all();
}

JNIEXPORT jlong JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeInit(
        JNIEnv* env, jclass type, jint inputSampleRate, jint inputChannelNum,
        jint outputSampleRate, jint outputChannelNum) {
    return toJ(new AudioResampler(kOutputSampleFormat, inputSampleRate, inputChannelNum,
                                  kOutputSampleFormat, outputSampleRate, outputChannelNum));
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeResample(
        JNIEnv* env, jclass type, jlong handle, jbyteArray inputBuffer_, jint inputSize,
        jbyteArray outputBuffer_) {
    jbyte* inputBuffer = env->GetByteArrayElements(inputBuffer_, NULL);
    jbyte* outputBuffer = env->GetByteArrayElements(outputBuffer_, NULL);

    int resampled = fromJ(AudioResampler, handle)->Resample(
            reinterpret_cast<void**>(&inputBuffer), inputSize,
            reinterpret_cast<void**>(&outputBuffer));

    env->ReleaseByteArrayElements(inputBuffer_, inputBuffer, 0);
    env->ReleaseByteArrayElements(outputBuffer_, outputBuffer, 0);

    return resampled;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioResampler, handle);
}


JNIEXPORT jlong JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeInit(
        JNIEnv* env, jclass type, jstring filepath_) {
    const char* filepath = env->GetStringUTFChars(filepath_, 0);

    std::string path(filepath);
    jlong handle = toJ(new AudioFileDecoder(path));

    env->ReleaseStringUTFChars(filepath_, filepath);

    return handle;
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeGetSampleRate(
        JNIEnv* env, jclass type, jlong handle) {
    return fromJ(AudioFileDecoder, handle)->sample_rate();
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeGetChannelNum(
        JNIEnv* env, jclass type, jlong handle) {
    return fromJ(AudioFileDecoder, handle)->channel_num();
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeConsume(JNIEnv* env, jclass type,
                                                                  jlong handle, jbyteArray buffer_,
                                                                  jint samples) {
    jbyte* buffer = env->GetByteArrayElements(buffer_, NULL);

    AudioFileDecoder* decoder = fromJ(AudioFileDecoder, handle);
    int consumed = decoder->Consume(reinterpret_cast<void**>(&buffer), samples);

    env->ReleaseByteArrayElements(buffer_, buffer, 0);

    return consumed;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioFileDecoder, handle);
}


JNIEXPORT jlong JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeInit(
        JNIEnv* env, jclass type, jstring filepath_, jint outputSampleRate,
        jint outputChannelNum, jint frameDurationMs) {
    const char* filepath = env->GetStringUTFChars(filepath_, 0);

    std::string path(filepath);
    AudioFileSource* source = new AudioFileSource(0, path, outputSampleRate, outputChannelNum,
                                                  frameDurationMs, 1);

    env->ReleaseStringUTFChars(filepath_, filepath);

    return toJ(source);
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeGetInputSampleRate(
        JNIEnv* env, jclass type, jlong handle) {
    AudioFileSource* source = fromJ(AudioFileSource, handle);
    return source->input_sample_rate();
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeGetInputChannelNum(
        JNIEnv* env, jclass type, jlong handle) {
    AudioFileSource* source = fromJ(AudioFileSource, handle);
    return source->input_channel_num();
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeRead(
        JNIEnv* env, jclass type, jlong handle, jbyteArray buffer_) {
    jbyte* buffer = env->GetByteArrayElements(buffer_, NULL);

    AudioFileSource* source = fromJ(AudioFileSource, handle);
    int read = source->Read(reinterpret_cast<void**>(&buffer));

    env->ReleaseByteArrayElements(buffer_, buffer, 0);

    return read;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioFileSource, handle);
}


JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_nativeMix(
        JNIEnv* env, jclass type, jlong handle, jbyteArray buffer_) {
    jbyte* buffer = env->GetByteArrayElements(buffer_, NULL);

    const auto& ref = ::djinni::objectFromHandleAddress<AudioMixerApi>(handle);
    int32_t size = reinterpret_cast<AudioMixer*>(ref.get())->Mix(reinterpret_cast<void*>(buffer));

    env->ReleaseByteArrayElements(buffer_, buffer, 0);
    return size;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_nativeAddRecordedData(
        JNIEnv* env, jclass type, jlong handle, jint j_ssrc, jbyteArray data_, jint size) {
    jbyte* data = env->GetByteArrayElements(data_, NULL);

    const auto& ref = ::djinni::objectFromHandleAddress<AudioMixerApi>(handle);
    reinterpret_cast<AudioMixer*>(ref.get())->AddRecordedData(::djinni::I32::toCpp(env, j_ssrc), data, size);

    env->ReleaseByteArrayElements(data_, data, 0);
}

}
}

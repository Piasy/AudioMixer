//
// Created by Piasy on 02/09/2017.
//

#include <string>
#include <jni.h>

#include <sdk/android/src/jni/jni_helpers.h>
#include <rtc_base/logging.h>

#include "avx_helper.h"
#include "audio_mixer.h"
#include "audio_resampler.h"
#include "audio_file_source.h"

#define toJ(handle) webrtc::jni::jlongFromPointer((handle))
#define fromJ(type, handle) reinterpret_cast<type *>((handle))

extern "C" {

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_globalInitializeFFMPEG(JNIEnv* env, jclass type) {
    av_register_all();
}

JNIEXPORT jlong JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_nativeInit(
        JNIEnv* env, jclass type) {
    return toJ(new AudioMixer());
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_nativeMix(
        JNIEnv* env, jclass type, jlong handle, jbyteArray buffer_) {
    jbyte* buffer = env->GetByteArrayElements(buffer_, NULL);

    int size = fromJ(AudioMixer, handle)->Mix(reinterpret_cast<void*>(buffer));

    env->ReleaseByteArrayElements(buffer_, buffer, 0);
    return size;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioMixer, handle);
}

JNIEXPORT jlong JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeInit(
        JNIEnv* env, jclass type, jint inputChannelNum, jint inputSampleRate,
        jint inputSamples, jint outputChannelNum, jint outputSampleRate) {
    return toJ(new AudioResampler(AV_SAMPLE_FMT_S16, inputSampleRate, inputChannelNum, inputSamples,
                                  AV_SAMPLE_FMT_S16, outputSampleRate, outputChannelNum));
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeResample(
        JNIEnv* env, jclass type, jlong handle, jbyteArray inputBuffer_, jint inputSize,
        jbyteArray outputBuffer_) {
    jbyte* inputBuffer = env->GetByteArrayElements(inputBuffer_, NULL);
    jbyte* outputBuffer = env->GetByteArrayElements(outputBuffer_, NULL);

    int resampled = fromJ(AudioResampler, handle)->Resample(
            reinterpret_cast<void**>(&inputBuffer), inputSize,
            reinterpret_cast<void*>(outputBuffer));

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
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeInit(
        JNIEnv* env, jclass type, jint ssrc, jstring filepath_, jint outputSampleRate,
        jint outputChannelNum) {
    const char* filepath = env->GetStringUTFChars(filepath_, 0);

    std::string path(filepath);
    AudioFileSource* source = new AudioFileSource(ssrc, path, outputSampleRate, outputChannelNum);

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
        JNIEnv* env, jclass type, jlong handle, jbyteArray buffer_, jint samples) {
    jbyte* buffer = env->GetByteArrayElements(buffer_, NULL);

    AudioFileSource* source = fromJ(AudioFileSource, handle);
    int read = source->Read(reinterpret_cast<void*>(buffer), samples);

    env->ReleaseByteArrayElements(buffer_, buffer, 0);

    return read;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileSource_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioFileSource, handle);
}

}

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
#include "audio_file_decoder.h"

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
        JNIEnv* env, jclass type, jlong handle, jbyteArray buf_) {
    jbyte* buf = env->GetByteArrayElements(buf_, NULL);

    int size = fromJ(AudioMixer, handle)->mix(reinterpret_cast<void*>(buf));

    env->ReleaseByteArrayElements(buf_, buf, 0);
    return size;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioMixer_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioMixer, handle);
}

JNIEXPORT jlong JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeInit(
        JNIEnv* env, jclass type, jint bytesPerSample, jint inChannelNum, jint inSampleRate,
        jint inSamples, jint outChannelNum, jint outSampleRate) {
    return toJ(new AudioResampler(bytesPerSample, inChannelNum, inSampleRate, inSamples,
                                  outChannelNum, outSampleRate));
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioResampler_nativeResample(
        JNIEnv* env, jclass type, jlong handle, jbyteArray inData_, jint inLen,
        jbyteArray outData_) {
    jbyte* inData = env->GetByteArrayElements(inData_, NULL);
    jbyte* outData = env->GetByteArrayElements(outData_, NULL);

    int resampled = fromJ(AudioResampler, handle)->resample(
            reinterpret_cast<const void*>(inData), inLen, reinterpret_cast<void*>(outData));

    env->ReleaseByteArrayElements(inData_, inData, 0);
    env->ReleaseByteArrayElements(outData_, outData, 0);

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
    AudioFileDecoder* decoder = new AudioFileDecoder(path);

    env->ReleaseStringUTFChars(filepath_, filepath);

    return toJ(decoder);
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeGetSampleRate(
        JNIEnv* env, jclass type, jlong handle) {
    AudioFileDecoder* decoder = fromJ(AudioFileDecoder, handle);
    return decoder->getSampleRate();
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeGetChannelNum(
        JNIEnv* env, jclass type, jlong handle) {
    AudioFileDecoder* decoder = fromJ(AudioFileDecoder, handle);
    return decoder->getChannelNum();
}

JNIEXPORT jint JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeConsume(
        JNIEnv* env, jclass type, jlong handle, jbyteArray buffer_, jint samples) {
    jbyte* buffer = env->GetByteArrayElements(buffer_, NULL);

    AudioFileDecoder* decoder = fromJ(AudioFileDecoder, handle);
    int decoded = decoder->consume(reinterpret_cast<void*>(buffer), samples);

    env->ReleaseByteArrayElements(buffer_, buffer, 0);

    return decoded;
}

JNIEXPORT void JNICALL
Java_com_github_piasy_audio_1mixer_AudioFileDecoder_nativeDestroy(
        JNIEnv* env, jclass type, jlong handle) {
    delete fromJ(AudioFileDecoder, handle);
}

}

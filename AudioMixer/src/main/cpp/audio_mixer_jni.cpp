//
// Created by Piasy on 02/09/2017.
//

#include <string>
#include <jni.h>

#include <sdk/android/src/jni/jni_helpers.h>
#include <rtc_base/logging.h>

#include "audio_mixer.h"
#include "audio_resampler.h"

#define toJ(handle) webrtc::jni::jlongFromPointer((handle))
#define fromJ(type, handle) reinterpret_cast<type *>((handle))

extern "C" {

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
        JNIEnv* env, jclass type, jint inChannelNum, jint inSampleRate, jint inSamples,
        jint outChannelNum, jint outSampleRate) {
    return toJ(new AudioResampler(inChannelNum, inSampleRate, inSamples, outChannelNum,
                                  outSampleRate));
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

}

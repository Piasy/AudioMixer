package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 25/12/2016.
 */

public class AudioMixer {

    private static boolean sInitialized;

    private final AudioBuffer mBuffer;

    private long mNativeHandle;

    public AudioMixer(MixerConfig config) {
        mNativeHandle = nativeInit(config);
        mBuffer = new AudioBuffer(new byte[AudioBuffer.MAX_BUF_SIZE], 0);
    }

    public static synchronized void globalInitialize() {
        if (!sInitialized) {
            sInitialized = true;

            System.loadLibrary("c++_shared");
            System.loadLibrary("avcodec");
            System.loadLibrary("avformat");
            System.loadLibrary("avutil");
            System.loadLibrary("swresample");
            System.loadLibrary("audio_mixer");

            globalInitializeFFMPEG();
        }
    }

    private static native void globalInitializeFFMPEG();

    private static native long nativeInit(MixerConfig config);

    private static native int nativeMix(long handle, byte[] buf);

    private static native void nativeDestroy(long handle);

    public AudioBuffer mix() {
        return mBuffer.setSize(
                nativeMix(mNativeHandle, mBuffer.getBuffer())
        );
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

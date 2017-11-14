package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 25/12/2016.
 */

public class AudioMixer {

    private static boolean sInitialized;

    private long mNativeHandle;
    private byte[] mBuffer;

    public AudioMixer() {
        mNativeHandle = nativeInit();
        mBuffer = new byte[BufferInfo.MAX_BUF_SIZE];
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

    private static native long nativeInit();

    private static native int nativeMix(long nativeClient, byte[] buf);

    private static native void nativeDestroy(long nativeClient);

    public byte[] mix() {
        int size = nativeMix(mNativeHandle, mBuffer);
        return mBuffer;
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 13/11/2017.
 */

public class AudioFileDecoder {
    private long mNativeHandle;
    private BufferInfo mBuffer;

    public AudioFileDecoder(String filepath) {
        mNativeHandle = nativeInit(filepath);
        mBuffer = new BufferInfo(new byte[BufferInfo.MAX_BUF_SIZE], 0);
    }

    private static native long nativeInit(String filepath);

    private static native int nativeGetSampleRate(long handle);

    private static native int nativeGetChannelNum(long handle);

    private static native int nativeConsume(long handle, byte[] buffer, int samples);

    private static native void nativeDestroy(long handle);

    public int getSampleRate() {
        return nativeGetSampleRate(mNativeHandle);
    }

    public int getChannelNum() {
        return nativeGetChannelNum(mNativeHandle);
    }

    public BufferInfo consume(int samples) {
        mBuffer.setSize(nativeConsume(mNativeHandle, mBuffer.getBuffer(), samples));
        return mBuffer;
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

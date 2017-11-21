package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 13/11/2017.
 */

public class AudioFileSource {
    // real value of AVERROR_EOF
    public static final int ERR_EOF = -541478725;

    private static int sSsrc = 0;

    private final BufferInfo mBuffer;

    private long mNativeHandle;

    public AudioFileSource(String filepath, int outputSampleRate, int outputChannelNum) {
        mNativeHandle = nativeInit(ssrc(), filepath, outputSampleRate, outputChannelNum);
        mBuffer = new BufferInfo(new byte[BufferInfo.MAX_BUF_SIZE], 0);
    }

    private static synchronized int ssrc() {
        sSsrc++;
        return sSsrc;
    }

    private static native long nativeInit(int ssrc, String filepath, int outputSampleRate,
            int outputChannelNum);

    private static native int nativeGetInputSampleRate(long handle);

    private static native int nativeGetInputChannelNum(long handle);

    private static native int nativeRead(long handle, byte[] buffer, int samples);

    private static native void nativeDestroy(long handle);

    public int getInputSampleRate() {
        return nativeGetInputSampleRate(mNativeHandle);
    }

    public int getInputChannelNum() {
        return nativeGetInputChannelNum(mNativeHandle);
    }

    public BufferInfo read(int samples) {
        mBuffer.setSize(nativeRead(mNativeHandle, mBuffer.getBuffer(), samples));
        return mBuffer;
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

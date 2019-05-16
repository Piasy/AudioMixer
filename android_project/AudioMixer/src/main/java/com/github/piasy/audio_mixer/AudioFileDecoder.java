package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 05/11/2017.
 *
 * Usage:
 *
 * <pre>{@code 
 * AudioFileDecoder decoder = new AudioFileDecoder(filepath, msPerBuf);
 *
 * while (true) {
 *     AudioBuffer buffer = decoder.consume();
 *     if (buffer.getSize() > 0) {
 *         audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
 *     } else {
 *         exitCode = buffer.getSize();
 *         break;
 *     }
 * }
 *
 * Log.d(TAG, "decode finish: " + exitCode);
 *
 * decoder.destroy();
 * }</pre>
 */

public class AudioFileDecoder {
    private final AudioBuffer mBuffer;
    private final int mSamplesPerBuf;

    private long mNativeHandle;

    public AudioFileDecoder(String filepath, int frameDurationMs) {
        mNativeHandle = nativeInit(filepath);

        mSamplesPerBuf = getSampleRate() / (1000 / frameDurationMs);
        int bufferSize = mSamplesPerBuf * getChannelNum() * AudioMixer.SAMPLE_SIZE;
        mBuffer = new AudioBuffer(new byte[bufferSize], bufferSize);
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

    public AudioBuffer consume() {
        return mBuffer.setSize(
                nativeConsume(mNativeHandle, mBuffer.getBuffer(), mSamplesPerBuf)
        );
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

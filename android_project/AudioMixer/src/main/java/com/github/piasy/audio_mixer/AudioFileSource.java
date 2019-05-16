package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 13/11/2017.
 *
 * Usage:
 *
 * <pre>{@code 
 * AudioFileSource source = new AudioFileSource(filepath, outputSampleRate, outputChannelNum,
 *         msPerBuf);
 *
 * while (true) {
 *     AudioBuffer buffer = source.read();
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
 * source.destroy();
 * }</pre>
 */

public class AudioFileSource {
    // real value of AVERROR_EOF
    public static final int ERR_EOF = -541478725;

    private final AudioBuffer mBuffer;

    private long mNativeHandle;

    public AudioFileSource(String filepath, int outputSampleRate, int outputChannelNum,
            int frameDurationMs) {
        mNativeHandle = nativeInit(filepath, outputSampleRate, outputChannelNum, frameDurationMs);
        mBuffer = new AudioBuffer(new byte[AudioMixer.MAX_BUF_SIZE], 0);
    }

    private static native long nativeInit(String filepath, int outputSampleRate,
            int outputChannelNum, int frameDurationMs);

    private static native int nativeGetInputSampleRate(long handle);

    private static native int nativeGetInputChannelNum(long handle);

    private static native int nativeRead(long handle, byte[] buffer);

    private static native void nativeDestroy(long handle);

    public int getInputSampleRate() {
        return nativeGetInputSampleRate(mNativeHandle);
    }

    public int getInputChannelNum() {
        return nativeGetInputChannelNum(mNativeHandle);
    }

    public AudioBuffer read() {
        mBuffer.setSize(nativeRead(mNativeHandle, mBuffer.getBuffer()));
        return mBuffer;
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 05/11/2017.
 *
 * Usage:
 *
 * <pre>{@code 
 * AudioResampler resampler = new AudioResampler(inputSampleRate, inputChannelNum,
 *      outputSampleRate, outputChannelNum);
 * AudioBuffer inputBuffer = resampler.getInputBuffer();
 *
 * while ((read = inputStream.read(inputBuffer.getBuffer())) > 0) {
 *     inputBuffer.setSize(read);
 *     AudioBuffer outputBuffer = resampler.resample(inputBuffer);
 *     if (outputBuffer.getSize() > 0) {
 *         audioTrack.write(outputBuffer.getBuffer(), 0, outputBuffer.getSize());
 *     } else {
 *         Log.e(TAG, "resample error " + outputBuffer.getSize());
 *     }
 * }
 *
 * resampler.destroy();
 * }</pre>
 */

public class AudioResampler {
    private final AudioBuffer mInputBuffer;
    private final AudioBuffer mOutputBuffer;

    private long mNativeHandle;

    public AudioResampler(int inputSampleRate, int inputChannelNum, int outputSampleRate,
            int outputChannelNum, int frameDurationMs) {
        mNativeHandle = nativeInit(inputSampleRate, inputChannelNum, outputSampleRate,
                outputChannelNum);

        int inputSamplesPerBuf = inputSampleRate / (1000 / frameDurationMs);
        int inputBufferSize = inputSamplesPerBuf * inputChannelNum * AudioMixer.SAMPLE_SIZE;
        mInputBuffer = new AudioBuffer(new byte[inputBufferSize], inputBufferSize);

        int outputBufferSize = outputSampleRate / (1000 / frameDurationMs)
                               * outputChannelNum * AudioMixer.SAMPLE_SIZE;
        // there may have some delay in swr, so output buffer may be lager
        outputBufferSize *= 2;
        mOutputBuffer = new AudioBuffer(new byte[outputBufferSize], 0);
    }

    private static native long nativeInit(int inputSampleRate, int inputChannelNum,
            int outputSampleRate, int outputChannelNum);

    private static native int nativeResample(long handle, byte[] inputBuffer, int inputSize,
            byte[] outputBuffer);

    private static native void nativeDestroy(long handle);

    public AudioBuffer getInputBuffer() {
        return mInputBuffer;
    }

    public AudioBuffer resample(AudioBuffer inputBuffer) {
        return mOutputBuffer.setSize(
                nativeResample(mNativeHandle, inputBuffer.getBuffer(), inputBuffer.getSize(),
                        mOutputBuffer.getBuffer())
        );
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

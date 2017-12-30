package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 05/11/2017.
 *
 * Usage:
 *
 * <pre>
 * AudioResampler resampler = new AudioResampler(inputChannelNum, inputSampleRate,
 *      outputChannelNum, outputSampleRate);
 * BufferInfo inputBuffer = resampler.getInputBuffer();
 *
 * while ((read = inputStream.read(inputBuffer.getBuffer(), 0,
 *     inputBuffer.getBuffer().length)) > 0) {
 *     inputBuffer.setSize(read);
 *     BufferInfo outputBuffer = resampler.resample(inputBuffer);
 *     if (outputBuffer.getSize() > 0) {
 *         audioTrack.write(outputBuffer.getBuffer(), 0, outputBuffer.getSize());
 *     } else {
 *         Log.e("MainActivity", "resample error " + outputBuffer.getSize());
 *     }
 * }
 *
 * resampler.destroy();
 * <pre/>
 */

public class AudioResampler {
    private final BufferInfo mInputBuffer;
    private final BufferInfo mOutputBuffer;

    private long mNativeHandle;

    public AudioResampler(int inputChannelNum, int inputSampleRate, int outputChannelNum,
            int outputSampleRate) {
        int inputSamplesPerBuf = inputSampleRate / (1000 / BufferInfo.MS_PER_BUF);

        mNativeHandle = nativeInit(inputSampleRate, inputChannelNum, inputSamplesPerBuf,
                outputSampleRate, outputChannelNum);

        int inputBufferSize = inputSampleRate / (1000 / BufferInfo.MS_PER_BUF)
                              * inputChannelNum * BufferInfo.SAMPLE_SIZE;
        mInputBuffer = new BufferInfo(new byte[inputBufferSize], inputBufferSize);

        int outputBufferSize = outputSampleRate / (1000 / BufferInfo.MS_PER_BUF)
                               * outputChannelNum * BufferInfo.SAMPLE_SIZE;
        // there may have some delay in swr, so output buffer may be lager
        outputBufferSize *= 2;
        mOutputBuffer = new BufferInfo(new byte[outputBufferSize], 0);
    }

    private static native long nativeInit(int inputSampleRate, int inputChannelNum,
            int inputSamples, int outputSampleRate, int outputChannelNum);

    private static native int nativeResample(long handle, byte[] inputBuffer, int inputSize,
            byte[] outputBuffer);

    private static native void nativeDestroy(long handle);

    public BufferInfo getInputBuffer() {
        return mInputBuffer;
    }

    public BufferInfo resample(BufferInfo inputBuffer) {
        mOutputBuffer.setSize(
                nativeResample(mNativeHandle, inputBuffer.getBuffer(), inputBuffer.getSize(),
                        mOutputBuffer.getBuffer())
        );
        return mOutputBuffer;
    }

    public void destroy() {
        nativeDestroy(mNativeHandle);
    }
}

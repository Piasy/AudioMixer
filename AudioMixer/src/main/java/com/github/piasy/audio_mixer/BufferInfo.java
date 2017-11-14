package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 15/11/2017.
 */
public class BufferInfo {
    // stay the same as webrtc::AudioMixerImpl::kFrameDurationInMs
    public static final int MS_PER_BUF = 10;
    public static final int MAX_BUF_SIZE = 7680;

    private byte[] buffer;
    private int size;

    BufferInfo(final byte[] buffer, final int size) {
        this.buffer = buffer;
        this.size = size;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public int getSize() {
        return size;
    }
}

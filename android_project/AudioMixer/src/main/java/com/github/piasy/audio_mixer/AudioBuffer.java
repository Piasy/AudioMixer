package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 15/11/2017.
 */
public class AudioBuffer {
    public static final int MAX_BUF_SIZE = 7680;
    public static final int SAMPLE_SIZE = 2;

    private byte[] buffer;
    private int size;

    AudioBuffer(final byte[] buffer, final int size) {
        this.buffer = buffer;
        this.size = size;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public int getSize() {
        return size;
    }

    public AudioBuffer setSize(int size) {
        this.size = size;
        return this;
    }
}

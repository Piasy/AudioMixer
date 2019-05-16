package com.github.piasy.audio_mixer;

/**
 * Created by Piasy{github.com/Piasy} on 25/12/2016.
 */

public class AudioMixer extends AudioMixerApi {

    private static boolean sInitialized;

    private final AudioMixerApi.CppProxy mNativeMixer;
    private final AudioBuffer mBuffer;

    public AudioMixer(MixerConfig config) {
        mNativeMixer = (AudioMixerApi.CppProxy) AudioMixerApi.create(config);
        mBuffer = new AudioBuffer(new byte[MAX_BUF_SIZE], 0);
    }

    public static synchronized void globalInitialize() {
        if (!sInitialized) {
            sInitialized = true;

            System.loadLibrary("c++_shared");
            System.loadLibrary("audio_mixer");

            globalInitializeFFmpeg();
        }
    }

    private static native void globalInitializeFFmpeg();

    private static native int nativeMix(long handle, byte[] buf);

    private static native int nativeAddRecordedData(long handle, int ssrc, byte[] data, int size);

    @Override
    public void updateVolume(final int ssrc, final float volume) {
        mNativeMixer.updateVolume(ssrc, volume);
    }

    @Override
    public boolean addSource(final MixerSource source) {
        return mNativeMixer.addSource(source);
    }

    @Override
    public boolean removeSource(final int ssrc) {
        return mNativeMixer.removeSource(ssrc);
    }

    public AudioBuffer mix() {
        return mBuffer.setSize(
                nativeMix(mNativeMixer.nativeRef, mBuffer.getBuffer())
        );
    }

    public void addRecordedData(final int ssrc, byte[] data, int size) {
        nativeAddRecordedData(mNativeMixer.nativeRef, ssrc, data, size);
    }

    public void destroy() {
        mNativeMixer._djinni_private_destroy();
    }
}

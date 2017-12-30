/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Piasy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package com.github.piasy.audio_mixer;

import android.Manifest;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import butterknife.ButterKnife;
import butterknife.OnClick;
import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.RuntimePermissions;

@RuntimePermissions
public class MainActivity extends AppCompatActivity {

    public static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ButterKnife.bind(this);
    }

    @OnClick(R.id.mBtnMix)
    void mix() {
        MainActivityPermissionsDispatcher.doMixWithPermissionCheck(MainActivity.this);
    }

    @OnClick(R.id.mBtnDecode)
    void decode() {
        MainActivityPermissionsDispatcher.doDecodeWithPermissionCheck(MainActivity.this);
    }

    @Override
    public void onRequestPermissionsResult(final int requestCode,
            @NonNull final String[] permissions,
            @NonNull final int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        MainActivityPermissionsDispatcher.onRequestPermissionsResult(this, requestCode,
                grantResults);
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doMix() {
        new Thread(() -> {
            int minBufferSize = AudioTrack.getMinBufferSize(48000,
                    AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, 48000,
                            AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, BufferInfo.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            AudioMixer mixer = new AudioMixer();
            boolean mixing = true;
            while (mixing) {
                byte[] data = mixer.mix();
                audioTrack.write(data, 0, 480 * 1 * 2);
            }

            Log.d(TAG, "mix finish");

            audioTrack.stop();
            mixer.destroy();
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doDecode() {
        new Thread(() -> {
            int sampleRate = 48000;
            int channelNum = 1;
            AudioFileSource source = new AudioFileSource("/sdcard/mp3/morning.mp3", sampleRate,
                    channelNum);

            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate,
                    channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                            channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, BufferInfo.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            int exitCode;
            while (true) {
                BufferInfo bufferInfo = source.read(sampleRate / (1000 / BufferInfo.MS_PER_BUF));
                if (bufferInfo.getSize() > 0) {
                    audioTrack.write(bufferInfo.getBuffer(), 0, bufferInfo.getSize());
                } else {
                    exitCode = bufferInfo.getSize();
                    break;
                }
            }

            if (exitCode == AudioFileSource.ERR_EOF) {
                Log.d(TAG, "decode finish");
            } else {
                Log.e(TAG, "decode error: " + exitCode);
            }

            audioTrack.stop();
            source.destroy();
        }).start();
    }
}

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
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
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

    @OnClick(R.id.mBtnResample)
    void resample() {
        MainActivityPermissionsDispatcher.doResampleWithPermissionCheck(MainActivity.this);
    }

    @OnClick(R.id.mBtnDecodeMono)
    void decodeMono() {
        MainActivityPermissionsDispatcher.doDecodeMonoWithPermissionCheck(MainActivity.this);
    }

    @OnClick(R.id.mBtnDecodeAny)
    void decodeAny() {
        MainActivityPermissionsDispatcher.doDecodeAnyWithPermissionCheck(MainActivity.this);
    }

    @OnClick(R.id.mBtnMix)
    void mix() {
        MainActivityPermissionsDispatcher.doMixWithPermissionCheck(MainActivity.this);
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
    void doResample() {
        new Thread(() -> {
            int inputSampleRate = 44100;
            int inputChannelNum = 2;
            int outputSampleRate = 48000;
            int outputChannelNum = 1;

            AudioResampler resampler = new AudioResampler(inputSampleRate, inputChannelNum,
                    outputSampleRate, outputChannelNum);
            AudioBuffer inputBuffer = resampler.getInputBuffer();

            int minBufferSize = AudioTrack.getMinBufferSize(outputSampleRate,
                    outputChannelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, outputSampleRate,
                            outputChannelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioBuffer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            try {
                FileInputStream inputStream = new FileInputStream("/sdcard/mp3/morning.raw");

                int read;
                while ((read = inputStream.read(inputBuffer.getBuffer())) > 0) {
                    inputBuffer.setSize(read);
                    AudioBuffer outputBuffer = resampler.resample(inputBuffer);

                    if (outputBuffer.getSize() > 0) {
                        audioTrack.write(outputBuffer.getBuffer(), 0, outputBuffer.getSize());
                    } else {
                        Log.e(TAG, "resample error: " + outputBuffer.getSize());
                        break;
                    }
                }

                inputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

            audioTrack.stop();
            resampler.destroy();

            Log.d(TAG, "resample finish");
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doDecodeMono() {
        new Thread(() -> {
            AudioFileDecoder decoder = new AudioFileDecoder("/sdcard/mp3/morning-mono-16k.mp3",
                    AudioBuffer.MS_PER_BUF);

            int minBufferSize = AudioTrack.getMinBufferSize(decoder.getSampleRate(),
                    decoder.getChannelNum() == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, decoder.getSampleRate(),
                            decoder.getChannelNum() == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioBuffer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            int exitCode;
            while (true) {
                AudioBuffer buffer = decoder.consume();
                if (buffer.getSize() > 0) {
                    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
                } else {
                    exitCode = buffer.getSize();
                    break;
                }
            }

            Log.d(TAG, "decode finish: " + exitCode);

            audioTrack.stop();
            decoder.destroy();
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doDecodeAny() {
        new Thread(() -> {
            int sampleRate = 48000;
            int channelNum = 1;
            AudioFileSource source = new AudioFileSource("/sdcard/mp3/morning.mp3", sampleRate,
                    channelNum, AudioBuffer.MS_PER_BUF);

            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate,
                    channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                            channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioBuffer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            int exitCode;
            while (true) {
                AudioBuffer buffer = source.read();
                if (buffer.getSize() > 0) {
                    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
                } else {
                    exitCode = buffer.getSize();
                    break;
                }
            }

            Log.d(TAG, "decode finish: " + exitCode);

            audioTrack.stop();
            source.destroy();
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doMix() {
        new Thread(() -> {
            int sampleRate = 48000;
            int channelNum = 1;

            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate,
                    channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                            channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioBuffer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            AudioMixer mixer = new AudioMixer(new MixerConfig(
                    new ArrayList<>(Arrays.asList(
                            "/sdcard/mp3/morning.mp3", "/sdcard/mp3/lion.mp3",
                            "/sdcard/mp3/iamyou.mp3"
                    )),
                    sampleRate, channelNum
            ));

            int exitCode;
            while (true) {
                AudioBuffer buffer = mixer.mix();
                if (buffer.getSize() > 0) {
                    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
                } else {
                    exitCode = buffer.getSize();
                    break;
                }
            }

            // actually can't reach there, mixer never ends
            Log.d(TAG, "mix finish: " + exitCode);

            audioTrack.stop();
            mixer.destroy();
        }).start();
    }
}

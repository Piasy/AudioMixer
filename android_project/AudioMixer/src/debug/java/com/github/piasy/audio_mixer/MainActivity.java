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
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import butterknife.ButterKnife;
import butterknife.OnClick;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.RuntimePermissions;

@RuntimePermissions
public class MainActivity extends AppCompatActivity {

    public static final String TAG = "MainActivity";

    private volatile boolean mRunning = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ButterKnife.bind(this);
    }

    @Override
    protected void onPause() {
        mRunning = false;

        super.onPause();
    }

    @Override
    public void onRequestPermissionsResult(final int requestCode,
            @NonNull final String[] permissions,
            @NonNull final int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        MainActivityPermissionsDispatcher.onRequestPermissionsResult(this, requestCode,
                grantResults);
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

    @OnClick(R.id.mBtnRecordAndMix)
    void recordAndMix() {
        MainActivityPermissionsDispatcher.doRecordAndMixWithPermissionCheck(MainActivity.this);
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doResample() {
        mRunning = true;
        new Thread(() -> {
            int inputSampleRate = 44100;
            int inputChannelNum = 2;
            int outputSampleRate = 48000;
            int outputChannelNum = 1;
            int frameDurationMs = 10;

            AudioResampler resampler = new AudioResampler(inputSampleRate, inputChannelNum,
                    outputSampleRate, outputChannelNum, frameDurationMs);
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
                            Math.max(minBufferSize, AudioMixer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            try {
                FileInputStream inputStream = new FileInputStream("/sdcard/mp3/morning.raw");

                int read;
                while ((read = inputStream.read(inputBuffer.getBuffer())) > 0 && mRunning) {
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

            Log.e(TAG, "resample finish");
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doDecodeMono() {
        mRunning = true;
        new Thread(() -> {
            int frameDurationMs = 10;
            AudioFileDecoder decoder = new AudioFileDecoder("/sdcard/mp3/morning-mono-16k.mp3",
                    frameDurationMs);

            int minBufferSize = AudioTrack.getMinBufferSize(decoder.getSampleRate(),
                    decoder.getChannelNum() == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, decoder.getSampleRate(),
                            decoder.getChannelNum() == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioMixer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            int exitCode = 0;
            while (mRunning) {
                AudioBuffer buffer = decoder.consume();
                if (buffer.getSize() > 0) {
                    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
                } else {
                    exitCode = buffer.getSize();
                    break;
                }
            }

            audioTrack.stop();
            decoder.destroy();

            Log.e(TAG, "decode finish: " + exitCode);
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doDecodeAny() {
        mRunning = true;
        new Thread(() -> {
            int sampleRate = 48000;
            int channelNum = 1;
            int frameDurationMs = 10;
            AudioFileSource source = new AudioFileSource("/sdcard/mp3/morning-48k.mp3", sampleRate,
                    channelNum, frameDurationMs);

            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate,
                    channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                            channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioMixer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            int exitCode = 0;
            while (mRunning) {
                AudioBuffer buffer = source.read();
                if (buffer.getSize() > 0) {
                    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
                } else {
                    exitCode = buffer.getSize();
                    break;
                }
            }

            audioTrack.stop();
            source.destroy();

            Log.e(TAG, "decode finish: " + exitCode);
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    })
    void doMix() {
        mRunning = true;
        new Thread(() -> {
            int sampleRate = 48000;
            int channelNum = 1;
            int frameDurationMs = 5;

            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate,
                    channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                            : AudioFormat.CHANNEL_OUT_STEREO,
                    AudioFormat.ENCODING_PCM_16BIT);
            AudioTrack audioTrack =
                    new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                            channelNum == 1 ? AudioFormat.CHANNEL_OUT_MONO
                                    : AudioFormat.CHANNEL_OUT_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            Math.max(minBufferSize, AudioMixer.MAX_BUF_SIZE),
                            AudioTrack.MODE_STREAM);
            audioTrack.play();

            AudioMixer mixer = new AudioMixer(new MixerConfig(
                    new ArrayList<>(Arrays.asList(
                            new MixerSource(MixerSource.TYPE_FILE, 1, 1,
                                    "/sdcard/mp3/morning-48k.mp3", 0, 0),
                            new MixerSource(MixerSource.TYPE_FILE, 2, 1,
                                    "/sdcard/mp3/lion-48k.mp3", 0, 0),
                            new MixerSource(MixerSource.TYPE_FILE, 3, 1,
                                    "/sdcard/mp3/iamyou-48k.mp3", 0, 0)
                    )),
                    sampleRate, channelNum, frameDurationMs
            ));

            int exitCode = 0;
            while (mRunning) {
                AudioBuffer buffer = mixer.mix();
                if (buffer.getSize() > 0) {
                    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
                } else {
                    exitCode = buffer.getSize();
                    break;
                }
            }

            audioTrack.stop();
            mixer.destroy();

            Log.e(TAG, "mix finish: " + exitCode);
        }).start();
    }

    @NeedsPermission({
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.RECORD_AUDIO,
    })
    void doRecordAndMix() {
        mRunning = true;
        new Thread(() -> {
            int sampleRate = 48000;
            int channelNum = 1;
            int frameDurationMs = 10;

            int bufSize = (sampleRate / (1000 / frameDurationMs)) * channelNum * 2;
            byte[] buf = new byte[bufSize];

            final int channel = channelNum == 1 ? AudioFormat.CHANNEL_IN_MONO
                    : AudioFormat.CHANNEL_IN_STEREO;
            final int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
            int minBufferSize = AudioRecord.getMinBufferSize(sampleRate, channel, audioFormat);
            AudioRecord recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                    sampleRate, channel, audioFormat,
                    Math.max(minBufferSize, AudioMixer.MAX_BUF_SIZE));
            recorder.startRecording();

            AudioMixer mixer = new AudioMixer(new MixerConfig(
                    new ArrayList<>(Arrays.asList(
                            new MixerSource(MixerSource.TYPE_FILE, 1, 1,
                                    "/sdcard/mp3/morning-48k.mp3", 0, 0),
                            new MixerSource(MixerSource.TYPE_RECORD, 2, 1, "", sampleRate,
                                    channelNum)
                    )),
                    sampleRate, channelNum, frameDurationMs
            ));

            int exitCode = 0;
            try {
                FileOutputStream mixerDump = new FileOutputStream(
                        "/sdcard/mp3/record_and_mix.pcm");

                while (mRunning) {
                    int read = recorder.read(buf, 0, bufSize);
                    mixer.addRecordedData(2, buf, read);
                    AudioBuffer buffer = mixer.mix();
                    if (buffer.getSize() > 0) {
                        mixerDump.write(buffer.getBuffer(), 0, buffer.getSize());
                    } else {
                        exitCode = buffer.getSize();
                        break;
                    }
                }
                mixerDump.close();
            } catch (Exception e) {
                e.printStackTrace();
            }

            recorder.stop();
            mixer.destroy();

            Log.e(TAG, "mix finish: " + exitCode);
        }).start();
    }
}

# AudioMixer

A cross-platform audio mixer, supports Android, iOS, macOS and Windows. Powered by WebRTC, FFmpeg and Djinni.

## Install

### Android

``` gradle
allprojects {
    repositories {
        maven {
            url  "http://dl.bintray.com/piasy/maven"
        }
    }
}

compile 'com.github.piasy:AudioMixer:1.0.0'
```

## Usage

### Android

Initialize:

``` java
AudioMixer.globalInitialize();
```

Create mixer:

``` java
AudioMixer mixer = new AudioMixer(new MixerConfig(
        new ArrayList<>(Arrays.asList(
                new MixerSource(MixerSource.TYPE_FILE, 1, 1,
                        "/sdcard/mp3/morning.mp3", 0, 0),
                new MixerSource(MixerSource.TYPE_RECORD, 2, 1, "", sampleRate,
                        channelNum)
        )),
        sampleRate, channelNum, frameDurationMs
));
```

Do mix with recorded data:

``` java
AudioBuffer buffer = mixer.addRecordedDataAndMix(buf, size);
```

Do mix with file only:

``` java
AudioBuffer buffer = mixer.mix();
```

Use the mixed audio data:

``` java
if (buffer.getSize() > 0) {
    audioTrack.write(buffer.getBuffer(), 0, buffer.getSize());
}
```

For more detailed info, please refer to [the source code](https://github.com/Piasy/AudioMixer/tree/master/android_project/AudioMixer/).

## Dependencies

+ FFmpeg: 3.4.2
+ WebRTC: #23505

## Development

+ Before run demo, push mp3 to sdcard: `adb push mp3 /sdcard/`
+ Generate sources: `./run_djinni.sh`
+ Extract libs: `./extract_libs.sh`

## Caveat

+ Due to the limitation of WebRTC, `frameDurationMs` must be 10ms, and we must mix 10ms's audio data each time;

## TODO

+ [ ] ffmpeg 4.0 audio decoder doesn't work
+ [ ] iOS
+ [ ] macOS
+ [ ] Windows

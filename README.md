# AudioMixer

A cross-platform audio mixer, supports Android, iOS, macOS and Windows. Powered by WebRTC, FFmpeg and Djinni.

## Install

### Android

[ ![Download](https://api.bintray.com/packages/piasy/maven/AudioMixer/images/download.svg) ](https://bintray.com/piasy/maven/AudioMixer/_latestVersion)

``` gradle
allprojects {
    repositories {
        maven {
            url  "http://dl.bintray.com/piasy/maven"
        }
    }
}

compile 'com.github.piasy:AudioMixer:1.0.2'
```

### iOS & macOS

Due to the file size limitation, publish with CocoaPods is difficult, so please [download the prebuilt `AudioMixer.framework`](https://github.com/Piasy/AudioMixer/releases) directly.

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
mixer.addRecordedData(2, buf, size);
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

### iOS & macOS

Initialize:

``` objc
#import <AudioMixer/AudioMixer.h>

[PYAAudioMixer globalInitializeFFmpeg];
```

Create mixer:

``` objc
#import <AudioMixer/AudioMixer.h>

NSArray* mixerSources = @[
    [PYAMixerSource
        mixerSourceWithType:PYAMixerSourceTypeFile
                       ssrc:1
                     volume:1
                       path:[self pathForFileName:@"morning.mp3"]
                 sampleRate:0
                 channelNum:0],
    [PYAMixerSource mixerSourceWithType:PYAMixerSourceTypeRecord
                                   ssrc:2
                                 volume:1
                                   path:@""
                             sampleRate:_sampleRate
                             channelNum:_channelNum],
];
PYAMixerConfig* config = [PYAMixerConfig mixerConfigWithSources:mixerSources
                                               outputSampleRate:_sampleRate
                                               outputChannelNum:_channelNum
                                                frameDurationMs:10];
_mixer = [[PYAAudioMixer alloc] initWithConfig:config];
```

Do mix with recorded data:

``` objc
[_mixer addRecordedData:2 data:_buffer size:mixerInputSize];
_mixedBuffer = [_mixer mix];
```

Do mix with file only:

``` objc
_mixedBuffer = [_mixer mix];
```

Use the mixed audio data:

``` objc
if (_mixedBuffer.size > 0) {
  write([_recordAndMixDumper fileDescriptor], _mixedBuffer.data,
        _mixedBuffer.size);
}
```

For more detailed info, please refer to [the source code](https://github.com/Piasy/AudioMixer/tree/master/ios_project/).

## Dependencies

+ FFmpeg: 3.4.2
+ WebRTC: #23794

## Development

+ Before run Android demo, push mp3 to sdcard: `adb push mp3 /sdcard/`
+ Generate sources: `./run_djinni.sh`
+ Extract libs: `./extract_libs.sh`

## Caveat

+ Due to the limitation of WebRTC, `frameDurationMs` must be 10ms, and we must mix 10ms's audio data each time;

## TODO

+ [ ] ffmpeg 4.0 audio decoder doesn't work
+ [x] iOS
+ [x] macOS
+ [ ] Windows

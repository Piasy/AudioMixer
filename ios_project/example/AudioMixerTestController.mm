//
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Piasy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//


#import <AudioMixer/AudioMixer.h>

#include <sdk/objc/Framework/Native/api/audio_device_module.h>
#include <media/engine/adm_helpers.h>

#include "audio_file_decoder.h"
#include "audio_file_source.h"
#include "audio_resampler.h"
#include "audio_mixer_global.h"

#import "AudioMixerTestController.h"

#if defined(WEBRTC_IOS)
#define kChannelNum 1
#else
#define kChannelNum 2
#endif

static const int32_t kBytesPerSample = 2;

class SimpleAudioCallback : public webrtc::AudioTransport {
public:
    SimpleAudioCallback(AudioMixerTestController* controller)
        : _controller(controller) {}
    ~SimpleAudioCallback() {}

    int32_t RecordedDataIsAvailable(
        const void* audioSamples, const size_t nSamples,
        const size_t nBytesPerSample, const size_t nChannels,
        const uint32_t samplesPerSec, const uint32_t totalDelayMS,
        const int32_t clockDrift, const uint32_t currentMicLevel,
        const bool keyPressed, uint32_t& newMicLevel) {
        return [_controller RecordedDataIsAvailable:audioSamples
                                           nSamples:nSamples
                                    nBytesPerSample:nBytesPerSample
                                          nChannels:nChannels
                                      samplesPerSec:samplesPerSec
                                       totalDelayMS:totalDelayMS
                                         clockDrift:clockDrift
                                    currentMicLevel:currentMicLevel
                                         keyPressed:keyPressed];
    }

    // Implementation has to setup safe values for all specified out parameters.
    int32_t NeedMorePlayData(const size_t nSamples,
                             const size_t nBytesPerSample,
                             const size_t nChannels,
                             const uint32_t samplesPerSec, void* audioSamples,
                             size_t& nSamplesOut,  // NOLINT
                             int64_t* elapsed_time_ms, int64_t* ntp_time_ms) {
        *elapsed_time_ms = 0;
        *ntp_time_ms = 0;
        nSamplesOut = [_controller NeedMorePlayData:nSamples
                                    nBytesPerSample:nBytesPerSample
                                          nChannels:nChannels
                                      samplesPerSec:samplesPerSec
                                       audioSamples:audioSamples];
        return 0;
    }

    // only used by chrome
    // Method to pull mixed render audio data from all active VoE channels.
    // The data will not be passed as reference for audio processing internally.
    void PullRenderData(int bits_per_sample, int sample_rate,
                        size_t number_of_channels, size_t number_of_frames,
                        void* audio_data, int64_t* elapsed_time_ms,
                        int64_t* ntp_time_ms) {}

private:
    AudioMixerTestController* _controller;
};

typedef NS_ENUM(NSInteger, TestType) {
    TEST_NONE,
    TEST_DECODE_MONO,
    TEST_RESAMPLE,
    TEST_DECODE_ANY,
    TEST_MIX,
    TEST_RECORD_AND_MIX,
};

@implementation AudioMixerTestController {
    rtc::scoped_refptr<webrtc::AudioDeviceModule> _adm;
    SimpleAudioCallback* _callback;

    int32_t _sampleRate;
    int32_t _channelNum;

    TestType _testType;

    audio_mixer::AudioFileDecoder* _decoder;
    audio_mixer::AudioResampler* _resampler;
    audio_mixer::AudioFileSource* _source;
    
    void* _buffer;
    PYAAudioBuffer* _mixedBuffer;
    int32_t _remainingData;

    PYAAudioMixer* _mixer;

    int32_t _resamplerInputSampleRate;
    int32_t _resamplerInputChannelNum;
    NSFileHandle* _resamplerReader;
    void* _resamplerInputBuffer;
    
    NSFileHandle* _recordAndMixDumper;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _testType = TEST_NONE;
    }
    return self;
}

- (void)doDecodeMono {
    NSLog(@"doDecodeMono");
    if (_testType != TEST_NONE) {
        NSLog(@"test already started: %ld", (long)_testType);
        return;
    }
    _testType = TEST_DECODE_MONO;

    _sampleRate = 16000;
    _channelNum = kChannelNum;
    _remainingData = 0;

    _decoder = new audio_mixer::AudioFileDecoder(
        [[self pathForFileName:@"morning-mono-16k.mp3"]
            cStringUsingEncoding:NSUTF8StringEncoding]);

    [self doStartTest];
}

- (void)deliverMonoDecodedData:(void*)buf numFrames:(int32_t)numFrames {
    _decoder->Consume(&buf, numFrames);
}

- (void)doResample {
    NSLog(@"doResample");
    if (_testType != TEST_NONE) {
        NSLog(@"test already started: %ld", (long)_testType);
        return;
    }
    _testType = TEST_RESAMPLE;

    _resamplerInputSampleRate = 44100;
    _resamplerInputChannelNum = 2;
    _sampleRate = 16000;
    _channelNum = kChannelNum;
    _remainingData = 0;

    _resampler = new audio_mixer::AudioResampler(
        audio_mixer::kOutputSampleFormat, _resamplerInputSampleRate,
        _resamplerInputChannelNum, audio_mixer::kOutputSampleFormat,
        _sampleRate, _channelNum);

    NSString* path = [self pathForFileName:@"morning.raw"];
    _resamplerReader = [NSFileHandle fileHandleForReadingAtPath:path];

    _buffer = malloc(7680);
    _resamplerInputBuffer = malloc(7680);

    [self doStartTest];
}

- (void)deliverResampledData:(void*)buf numFrames:(int32_t)numFrames {
    int32_t wantedSize = kBytesPerSample * _channelNum * numFrames;
    int32_t resampleInputSize = kBytesPerSample * _resamplerInputChannelNum *
                                _resamplerInputSampleRate / 100;
    int32_t resampleOutputSize =
        kBytesPerSample * _channelNum * _sampleRate / 100;

    int32_t bufWritePos = 0;
    if (_remainingData > 0) {
        int32_t size = MIN(_remainingData, wantedSize);
        memcpy(buf, ((int8_t*)_buffer) + (resampleOutputSize - _remainingData),
               size);
        _remainingData -= size;
        wantedSize -= size;
        bufWritePos += size;
    }
    while (wantedSize > 0) {
        read([_resamplerReader fileDescriptor], _resamplerInputBuffer,
             resampleInputSize);
        int32_t size = MIN(resampleOutputSize, wantedSize);

        _resampler->Resample(&_resamplerInputBuffer, resampleInputSize,
                             &_buffer);
        memcpy((int8_t*)buf + bufWritePos, _buffer, size);

        bufWritePos += size;
        wantedSize -= size;
        _remainingData = resampleOutputSize - size;
    }
}

- (void)doDecodeAny {
    NSLog(@"doDecodeAny");
    if (_testType != TEST_NONE) {
        NSLog(@"test already started: %ld", (long)_testType);
        return;
    }
    _testType = TEST_DECODE_ANY;

    _sampleRate = 48000;
    _channelNum = kChannelNum;
    _remainingData = 0;

    _source = new audio_mixer::AudioFileSource(
        1, [[self pathForFileName:@"morning-48k.mp3"]
               cStringUsingEncoding:NSUTF8StringEncoding],
        _sampleRate, _channelNum, 10, 1);
    _buffer = malloc(7680);

    [self doStartTest];
}

- (void)deliverAnyDecodedData:(void*)buf numFrames:(int32_t)numFrames {
    int32_t wantedSize = numFrames * _channelNum * kBytesPerSample;
    int32_t decodeOutputSize =
        kBytesPerSample * _channelNum * _sampleRate / 100;

    int32_t bufWritePos = 0;
    if (_remainingData > 0) {
        int32_t size = MIN(_remainingData, wantedSize);
        memcpy(buf, ((int8_t*)_buffer) + (decodeOutputSize - _remainingData),
               size);
        _remainingData -= size;
        wantedSize -= size;
        bufWritePos += size;
    }
    while (wantedSize > 0) {
        int read = _source->Read(&_buffer);
        int32_t size = MIN(read, wantedSize);
        memcpy((int8_t*)buf + bufWritePos, _buffer, size);
        bufWritePos += size;
        wantedSize -= size;
        _remainingData = read - size;
    }
}

- (void)doMix {
    NSLog(@"doMix");
    if (_testType != TEST_NONE) {
        NSLog(@"test already started: %ld", (long)_testType);
        return;
    }
    _testType = TEST_MIX;

    _sampleRate = 48000;
    _channelNum = kChannelNum;
    _remainingData = 0;

    NSArray* mixerSources = @[
        [PYAMixerSource
            mixerSourceWithType:PYAMixerSourceTypeFile
                           ssrc:1
                         volume:1
                           path:[self pathForFileName:@"morning-48k.mp3"]
                     sampleRate:0
                     channelNum:0],
        [PYAMixerSource mixerSourceWithType:PYAMixerSourceTypeFile
                                       ssrc:2
                                     volume:1
                                       path:[self pathForFileName:@"iamyou-48k.mp3"]
                                 sampleRate:0
                                 channelNum:0],
        [PYAMixerSource mixerSourceWithType:PYAMixerSourceTypeFile
                                       ssrc:3
                                     volume:1
                                       path:[self pathForFileName:@"lion-48k.mp3"]
                                 sampleRate:0
                                 channelNum:0],
    ];
    PYAMixerConfig* config = [PYAMixerConfig mixerConfigWithSources:mixerSources
                                                   outputSampleRate:_sampleRate
                                                   outputChannelNum:_channelNum
                                                    frameDurationMs:3];
    _mixer = [[PYAAudioMixer alloc] initWithConfig:config];

    [self doStartTest];
}

- (void)deliverMixedData:(void*)buf numFrames:(int32_t)numFrames {
    int32_t wantedSize = numFrames * _channelNum * kBytesPerSample;

    int32_t bufWritePos = 0;
    if (_remainingData > 0) {
        int32_t size = MIN(_remainingData, wantedSize);
        memcpy(buf, ((int8_t*)_mixedBuffer.data) +
                        (_mixedBuffer.size - _remainingData),
               size);
        _remainingData -= size;
        wantedSize -= size;
        bufWritePos += size;
    }
    while (wantedSize > 0) {
        _mixedBuffer = [_mixer mix];
        int32_t size = MIN(_mixedBuffer.size, wantedSize);
        memcpy((int8_t*)buf + bufWritePos, _mixedBuffer.data, size);
        bufWritePos += size;
        wantedSize -= size;
        _remainingData = _mixedBuffer.size - size;
    }
}

- (void)doRecordAndMix {
    NSLog(@"doRecordAndMix");
    if (_testType != TEST_NONE) {
        NSLog(@"test already started: %ld", (long)_testType);
        return;
    }
    _testType = TEST_RECORD_AND_MIX;

    _sampleRate = 48000;
    _channelNum = kChannelNum;
    _remainingData = 0;

    NSArray* mixerSources = @[
        [PYAMixerSource
            mixerSourceWithType:PYAMixerSourceTypeFile
                           ssrc:1
                         volume:1
                           path:[self pathForFileName:@"morning-48k.mp3"]
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
    _buffer = malloc(7860);

    NSString* dumpPath = [[self applicationDocumentsDirectory].path
        stringByAppendingPathComponent:@"record_and_mix.pcm"];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    [fileManager createFileAtPath:dumpPath contents:nil attributes:nil];
    _recordAndMixDumper = [NSFileHandle fileHandleForWritingAtPath:dumpPath];
    [_recordAndMixDumper seekToFileOffset:0];

    [self doStartTest];
}

- (void)deliverRecordAndMixedData:(const void*)buf numFrames:(int32_t)numFrames {
    int32_t recordedSize = numFrames * _channelNum * kBytesPerSample;
    int32_t mixerInputSize = kBytesPerSample * _channelNum * _sampleRate / 100;

    int32_t bufReadPos = 0;
    if (_remainingData > 0) {
        int32_t size = MIN(recordedSize, mixerInputSize - _remainingData);
        memcpy((int8_t*)_buffer + _remainingData, buf, size);
        if (size == mixerInputSize - _remainingData) {
            [_mixer addRecordedData:2 data:_buffer size:mixerInputSize];
            _mixedBuffer = [_mixer mix];
            write([_recordAndMixDumper fileDescriptor], _mixedBuffer.data,
                  _mixedBuffer.size);
            _remainingData = 0;
        }
        recordedSize -= size;
        bufReadPos += size;
    }
    while (recordedSize > 0) {
        int32_t size = MIN(recordedSize, mixerInputSize);
        memcpy(_buffer, (int8_t*)buf + bufReadPos, size);
        if (recordedSize < mixerInputSize) {
            _remainingData = recordedSize;
            break;
        }
        [_mixer addRecordedData:2 data:_buffer size:mixerInputSize];
        _mixedBuffer = [_mixer mix];
        write([_recordAndMixDumper fileDescriptor], _mixedBuffer.data,
              _mixedBuffer.size);
        bufReadPos += mixerInputSize;
        recordedSize -= mixerInputSize;
    }
}

- (void)doStartTest {
#if defined(WEBRTC_IOS)
    _adm = webrtc::CreateAudioDeviceModule();
#else
    _adm = webrtc::AudioDeviceModule::Create(
        webrtc::AudioDeviceModule::kPlatformDefaultAudio);
#endif
    webrtc::adm_helpers::Init(_adm);
    _callback = new SimpleAudioCallback(self);
    _adm->RegisterAudioCallback(_callback);

    if (_testType == TEST_RECORD_AND_MIX) {
        if (_adm->InitRecording() == 0) {
            _adm->StartRecording();
        } else {
            NSLog(@"start record fail");
        }
    } else {
        if (_adm->InitPlayout() == 0) {
            _adm->StartPlayout();
        } else {
            NSLog(@"start play fail");
        }
    }
}

- (void)doStopTest {
    NSLog(@"doStopTest");
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    if (_resamplerInputBuffer) {
        free(_resamplerInputBuffer);
        _resamplerInputBuffer = nullptr;
    }
    if (_decoder) {
        delete _decoder;
        _decoder = nullptr;
    }
    if (_resampler) {
        delete _resampler;
        _resampler = nullptr;
    }
    if (_source) {
        delete _source;
        _source = nullptr;
    }
    if (_resamplerReader) {
        [_resamplerReader closeFile];
        _resamplerReader = nil;
    }
    if (_recordAndMixDumper) {
        [_recordAndMixDumper closeFile];
        _recordAndMixDumper = nil;
    }

    if (_testType == TEST_RECORD_AND_MIX) {
        _adm->StopRecording();
    } else {
        _adm->StopPlayout();
    }
    _adm->Terminate();
    _adm->RegisterAudioCallback(nullptr);
    _adm = nullptr;

    _testType = TEST_NONE;
}

- (int32_t)RecordedDataIsAvailable:(const void*)audioSamples
                          nSamples:(size_t)nSamples
                   nBytesPerSample:(size_t)nBytesPerSample
                         nChannels:(size_t)nChannels
                     samplesPerSec:(uint32_t)samplesPerSec
                      totalDelayMS:(uint32_t)totalDelayMS
                        clockDrift:(int32_t)clockDrift
                   currentMicLevel:(uint32_t)currentMicLevel
                        keyPressed:(bool)keyPressed {
    [self deliverRecordAndMixedData:audioSamples numFrames:(int32_t)nSamples];
    return 0;
}

- (size_t)NeedMorePlayData:(size_t)nSamples
           nBytesPerSample:(size_t)nBytesPerSample
                 nChannels:(size_t)nChannels
             samplesPerSec:(uint32_t)samplesPerSec
              audioSamples:(void*)audioSamples {
    switch (_testType) {
        case TEST_DECODE_MONO:
            [self deliverMonoDecodedData:audioSamples
                               numFrames:(int32_t)nSamples];
            break;
        case TEST_RESAMPLE:
            [self deliverResampledData:audioSamples
                             numFrames:(int32_t)nSamples];
            break;
        case TEST_DECODE_ANY:
            [self deliverAnyDecodedData:audioSamples
                              numFrames:(int32_t)nSamples];
            break;
        case TEST_MIX:
            [self deliverMixedData:audioSamples numFrames:(int32_t)nSamples];
            break;
        default:
            return 0;
    }
    return nSamples;
}

- (NSString*)pathForFileName:(NSString*)fileName {
    NSArray* nameComponents = [fileName componentsSeparatedByString:@"."];
    if (nameComponents.count != 2) {
        return nil;
    }

    NSString* path = [[NSBundle mainBundle] pathForResource:nameComponents[0]
                                                     ofType:nameComponents[1]];
    return path;
}

- (NSURL*)applicationDocumentsDirectory {
    return [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory
                                                   inDomains:NSUserDomainMask]
        lastObject];
}

@end

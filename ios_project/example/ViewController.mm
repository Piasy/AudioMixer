//
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

#import "ViewController.h"
#import "PYAAudioMixer.h"
#import "PYAMixerConfig.h"
#import "PYAMixerSource.h"

#import "audio_file_decoder.h"
#import "audio_file_source.h"
#import "audio_resampler.h"
#import "audio_mixer_global.h"

// A VP I/O unit's bus 1 connects to input hardware (microphone).
static const AudioUnitElement kInputBus = 1;
// A VP I/O unit's bus 0 connects to output hardware (speaker).
static const AudioUnitElement kOutputBus = 0;
static const int32_t kBytesPerSample = 2;

OSStatus OnGetPlayoutData(void* in_ref_con, AudioUnitRenderActionFlags* flags,
                          const AudioTimeStamp* time_stamp, UInt32 bus_number,
                          UInt32 num_frames, AudioBufferList* io_data) {
    ViewController* controller = (__bridge ViewController*)(in_ref_con);
    return [controller notifyGetPlayoutData:flags
                                  timestamp:time_stamp
                                  busNumber:bus_number
                                  numFrames:num_frames
                                     ioData:io_data];
}

typedef NS_ENUM(NSInteger, TestType) {
    TEST_DECODE_MONO,
    TEST_RESAMPLE,
    TEST_DECODE_ANY,
    TEST_MIX,
};

@implementation ViewController {
    AudioUnit _audioUnit;

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
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.

    CGRect screen = [[UIScreen mainScreen] bounds];

    UIButton* decodeMono = [UIButton buttonWithType:UIButtonTypeSystem];
    [decodeMono setTitle:@"DecodeMono" forState:UIControlStateNormal];
    decodeMono.titleLabel.font = [UIFont systemFontOfSize:20.0f];
    decodeMono.frame = CGRectMake((screen.size.width - 200) / 2, 80, 200, 40);

    [decodeMono addTarget:self
                  action:@selector(doDecodeMono)
        forControlEvents:UIControlEventTouchUpInside];

    [self.view addSubview:decodeMono];

    UIButton* resample = [UIButton buttonWithType:UIButtonTypeSystem];
    [resample setTitle:@"Resample" forState:UIControlStateNormal];
    resample.titleLabel.font = [UIFont systemFontOfSize:20.0f];
    resample.frame = CGRectMake((screen.size.width - 200) / 2, 130, 200, 40);

    [resample addTarget:self
                  action:@selector(doResample)
        forControlEvents:UIControlEventTouchUpInside];

    [self.view addSubview:resample];

    UIButton* decodeAny = [UIButton buttonWithType:UIButtonTypeSystem];
    [decodeAny setTitle:@"DecodeAny" forState:UIControlStateNormal];
    decodeAny.titleLabel.font = [UIFont systemFontOfSize:20.0f];
    decodeAny.frame = CGRectMake((screen.size.width - 200) / 2, 180, 200, 40);

    [decodeAny addTarget:self
                  action:@selector(doDecodeAny)
        forControlEvents:UIControlEventTouchUpInside];

    [self.view addSubview:decodeAny];

    UIButton* mix = [UIButton buttonWithType:UIButtonTypeSystem];
    [mix setTitle:@"Mix" forState:UIControlStateNormal];
    mix.titleLabel.font = [UIFont systemFontOfSize:20.0f];
    mix.frame = CGRectMake((screen.size.width - 100) / 2, 230, 100, 40);

    [mix addTarget:self
                  action:@selector(doMix)
        forControlEvents:UIControlEventTouchUpInside];

    [self.view addSubview:mix];
}

- (void)doDecodeMono {
    _testType = TEST_DECODE_MONO;

    _sampleRate = 16000;
    _channelNum = 1;
    _remainingData = 0;

    _decoder = new audio_mixer::AudioFileDecoder(
        [[self pathForFileName:@"morning-mono-16k.mp3"]
            cStringUsingEncoding:NSUTF8StringEncoding]);

    [self doStartTest];
}

- (void)deliverMonoDecodedData:(void*)buf numFrames:(UInt32)numFrames {
    _decoder->Consume(&buf, numFrames);
}

- (void)doResample {
    _testType = TEST_RESAMPLE;

    _resamplerInputSampleRate = 44100;
    _resamplerInputChannelNum = 2;
    _sampleRate = 16000;
    _channelNum = 1;
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

- (void)deliverResampledData:(void*)buf numFrames:(UInt32)numFrames {
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
    _testType = TEST_DECODE_ANY;

    _sampleRate = 48000;
    _channelNum = 1;
    _remainingData = 0;

    _source = new audio_mixer::AudioFileSource(
        1, [[self pathForFileName:@"morning.mp3"]
               cStringUsingEncoding:NSUTF8StringEncoding],
        _sampleRate, _channelNum, 10, 1);
    _buffer = malloc(7680);

    [self doStartTest];
}

- (void)deliverAnyDecodedData:(void*)buf numFrames:(UInt32)numFrames {
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
    _testType = TEST_MIX;

    _sampleRate = 48000;
    _channelNum = 1;
    _remainingData = 0;

    NSArray* mixerSources = @[
        [PYAMixerSource
            mixerSourceWithType:PYAMixerSourceTypeFile
                           ssrc:1
                         volume:1
                           path:[self pathForFileName:@"morning.mp3"]
                     sampleRate:0
                     channelNum:0],
        [PYAMixerSource mixerSourceWithType:PYAMixerSourceTypeFile
                                       ssrc:2
                                     volume:1
                                       path:[self pathForFileName:@"iamyou.mp3"]
                                 sampleRate:0
                                 channelNum:0],
        [PYAMixerSource mixerSourceWithType:PYAMixerSourceTypeFile
                                       ssrc:3
                                     volume:1
                                       path:[self pathForFileName:@"lion.mp3"]
                                 sampleRate:0
                                 channelNum:0],
    ];
    PYAMixerConfig* config = [PYAMixerConfig mixerConfigWithSources:mixerSources
                                                   outputSampleRate:_sampleRate
                                                   outputChannelNum:_channelNum
                                                    frameDurationMs:10];
    _mixer = [[PYAAudioMixer alloc] initWithConfig:config];

    [self doStartTest];
}

- (void)deliverMixedData:(void*)buf numFrames:(UInt32)numFrames {
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

- (void)doStartTest {
    dispatch_async(
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            if ([self createPlayback] && [self initPlayback] &&
                [self startPlayback]) {
                NSLog(@"doStartTest success");
            }
        });
}

- (bool)createPlayback {
    AudioComponentDescription vpio_unit_description;
    vpio_unit_description.componentType = kAudioUnitType_Output;
    vpio_unit_description.componentSubType =
        kAudioUnitSubType_VoiceProcessingIO;
    vpio_unit_description.componentManufacturer = kAudioUnitManufacturer_Apple;
    vpio_unit_description.componentFlags = 0;
    vpio_unit_description.componentFlagsMask = 0;

    // Obtain an audio unit instance given the description.
    AudioComponent found_vpio_unit_ref =
        AudioComponentFindNext(nil, &vpio_unit_description);

    // Create a Voice Processing IO audio unit.
    OSStatus result = noErr;
    result = AudioComponentInstanceNew(found_vpio_unit_ref, &_audioUnit);
    if (result != noErr) {
        _audioUnit = nil;
        NSLog(@"AudioComponentInstanceNew failed. Error=%ld.", (long)result);
        return false;
    }

    // Enable output on the output scope of the output element.
    UInt32 enable_output = 1;
    result = AudioUnitSetProperty(_audioUnit, kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output, kOutputBus,
                                  &enable_output, sizeof(enable_output));
    if (result != noErr) {
        [self destroyPlayback];
        NSLog(@"Failed to enable output on output scope of output element. "
               "Error=%ld.",
              (long)result);
        return false;
    }

    // Specify the callback function that provides audio samples to the audio
    // unit.
    AURenderCallbackStruct render_callback;
    render_callback.inputProc = OnGetPlayoutData;
    render_callback.inputProcRefCon = (__bridge void* _Nullable)(self);
    result = AudioUnitSetProperty(
        _audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
        kOutputBus, &render_callback, sizeof(render_callback));
    if (result != noErr) {
        [self destroyPlayback];
        NSLog(@"Failed to specify the render callback on the output bus. "
               "Error=%ld.",
              (long)result);
        return false;
    }

    return true;
}

- (bool)initPlayback {
    OSStatus result = noErr;
    AudioStreamBasicDescription format = [self getFormat];
    UInt32 size = sizeof(format);

    // Set the format on the output scope of the input element/bus.
    result =
        AudioUnitSetProperty(_audioUnit, kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Output, kInputBus, &format, size);
    if (result != noErr) {
        NSLog(@"Failed to set format on output scope of input bus. "
               "Error=%ld.",
              (long)result);
        return false;
    }

    // Set the format on the input scope of the output element/bus.
    result =
        AudioUnitSetProperty(_audioUnit, kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Input, kOutputBus, &format, size);
    if (result != noErr) {
        NSLog(@"Failed to set format on input scope of output bus. "
               "Error=%ld.",
              (long)result);
        return false;
    }

    // Initialize the Voice Processing I/O unit instance.
    // Calls to AudioUnitInitialize() can fail if called back-to-back on
    // different ADM instances. The error message in this case is -66635 which
    // is
    // undocumented. Tests have shown that calling AudioUnitInitialize a second
    // time, after a short sleep, avoids this issue.
    // See webrtc:5166 for details.
    int failed_initalize_attempts = 0;
    result = AudioUnitInitialize(_audioUnit);
    while (result != noErr) {
        NSLog(@"Failed to initialize the Voice Processing I/O unit. "
               "Error=%ld.",
              (long)result);
        ++failed_initalize_attempts;
        if (failed_initalize_attempts == 3) {
            // Max number of initialization attempts exceeded, hence abort.
            NSLog(@"Too many initialization attempts.");
            return false;
        }
        NSLog(@"Pause 100ms and try audio unit initialization again...");
        [NSThread sleepForTimeInterval:0.1f];
        result = AudioUnitInitialize(_audioUnit);
    }
    if (result == noErr) {
        NSLog(@"Voice Processing I/O unit is now initialized.");
    }

    // AGC should be enabled by default for Voice Processing I/O units but it is
    // checked below and enabled explicitly if needed. This scheme is used
    // to be absolutely sure that the AGC is enabled since we have seen cases
    // where only zeros are recorded and a disabled AGC could be one of the
    // reasons why it happens.
    int agc_was_enabled_by_default = 0;
    UInt32 agc_is_enabled = 0;

    size = sizeof(UInt32);
    result = AudioUnitGetProperty(
        _audioUnit, kAUVoiceIOProperty_VoiceProcessingEnableAGC,
        kAudioUnitScope_Global, kInputBus, &agc_is_enabled, &size);

    if (result != noErr) {
        NSLog(@"Failed to get AGC state (1st attempt). "
               "Error=%ld.",
              (long)result);
    } else if (agc_is_enabled) {
        // Remember that the AGC was enabled by default. Will be used in UMA.
        agc_was_enabled_by_default = 1;
    } else {
        // AGC was initially disabled => try to enable it explicitly.
        UInt32 enable_agc = 1;
        result = AudioUnitSetProperty(
            _audioUnit, kAUVoiceIOProperty_VoiceProcessingEnableAGC,
            kAudioUnitScope_Global, kInputBus, &enable_agc, sizeof(enable_agc));
        if (result != noErr) {
            NSLog(@"Failed to enable the built-in AGC. "
                   "Error=%ld.",
                  (long)result);
        }

        size = sizeof(UInt32);
        result = AudioUnitGetProperty(
            _audioUnit, kAUVoiceIOProperty_VoiceProcessingEnableAGC,
            kAudioUnitScope_Global, kInputBus, &agc_is_enabled, &size);

        if (result != noErr) {
            NSLog(@"Failed to get AGC state (2nd attempt). "
                   "Error=%ld.",
                  (long)result);
        }
    }

    NSLog(@"BuiltInAGCWasEnabledByDefault: %d", agc_was_enabled_by_default);
    return true;
}

- (bool)startPlayback {
    OSStatus result = AudioOutputUnitStart(_audioUnit);
    if (result != noErr) {
        NSLog(@"Failed to start audio unit. Error=%ld", (long)result);
        return false;
    }
    
    NSLog(@"Started audio unit");
    return true;
}

- (AudioStreamBasicDescription)getFormat {
    // Set the application formats for input and output:
    // - use same format in both directions
    // - avoid resampling in the I/O unit by using the hardware sample rate
    // - linear PCM => noncompressed audio data format with one frame per packet
    // - no need to specify interleaving since only mono is supported
    AudioStreamBasicDescription format;
    format.mSampleRate = _sampleRate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags =
        kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mBytesPerPacket = kBytesPerSample;
    format.mFramesPerPacket = 1;  // uncompressed.
    format.mBytesPerFrame = kBytesPerSample;
    format.mChannelsPerFrame = _channelNum;
    format.mBitsPerChannel = 8 * kBytesPerSample;
    return format;
}

- (OSStatus)notifyGetPlayoutData:(AudioUnitRenderActionFlags*)flags
                       timestamp:(const AudioTimeStamp*)timestamp
                       busNumber:(UInt32)busNumber
                       numFrames:(UInt32)numFrames
                          ioData:(AudioBufferList*)ioData {
    AudioBuffer* audioBuffer = &ioData->mBuffers[0];

    switch (_testType) {
        case TEST_DECODE_MONO:
            [self deliverMonoDecodedData:audioBuffer->mData
                               numFrames:numFrames];
            break;
        case TEST_RESAMPLE:
            [self deliverResampledData:audioBuffer->mData numFrames:numFrames];
            break;
        case TEST_DECODE_ANY:
            [self deliverAnyDecodedData:audioBuffer->mData numFrames:numFrames];
            break;
        case TEST_MIX:
            [self deliverMixedData:audioBuffer->mData numFrames:numFrames];
            break;
        default:
            break;
    }

    return noErr;
}

- (bool)destroyPlayback {
    OSStatus result = AudioUnitUninitialize(_audioUnit);
    if (result != noErr) {
        NSLog(@"Failed to uninitialize audio unit. Error=%ld", (long)result);
        return false;
    }
    NSLog(@"Uninitialized audio unit.");
    return true;
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

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end

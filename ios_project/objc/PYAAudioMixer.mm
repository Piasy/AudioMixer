
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


#import "PYAAudioMixer.h"

#import "PYAMixerConfig+Private.h"
#import "PYAAudioMixerApi+Private.h"

#include "avx_helper.h"
#include "audio_mixer.h"

@implementation PYAAudioMixer {
    PYAAudioMixerApi* _nativeMixer;
    PYAAudioBuffer* _buffer;
}

+ (void)globalInitializeFFmpeg {
    av_register_all();
}

- (instancetype)initWithConfig:(PYAMixerConfig*)config {
    self = [super init];

    if (self) {
        _nativeMixer = [PYAAudioMixerApi create:config];
        _buffer = [[PYAAudioBuffer alloc] init];
    }

    return self;
}

- (void)updateVolume:(int32_t)ssrc volume:(float)volume {
    [_nativeMixer updateVolume:ssrc volume:volume];
}

- (BOOL)addSource:(PYAMixerSource*)source {
    return [_nativeMixer addSource:source];
}

- (BOOL)removeSource:(int32_t)ssrc {
    return [_nativeMixer removeSource:ssrc];
}

- (PYAAudioBuffer*)mix {
    audio_mixer::AudioMixer* nativeMixer =
        (audio_mixer::AudioMixer*)[_nativeMixer nativeMixer];
    if (nativeMixer) {
        _buffer.size = nativeMixer->Mix(_buffer.data);
    }

    return _buffer;
}

- (PYAAudioBuffer*)addRecordedData:(int32_t)ssrc
                              data:(void*)data
                              size:(int32_t)size {
    audio_mixer::AudioMixer* nativeMixer =
        (audio_mixer::AudioMixer*)[_nativeMixer nativeMixer];
    if (nativeMixer) {
        nativeMixer->AddRecordedData(ssrc, data, size);
    }

    return _buffer;
}
@end

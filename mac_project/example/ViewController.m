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
#import "AudioMixerTestController.h"

@implementation ViewController {
    AudioMixerTestController* _controller;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    CGSize windowSize = self.view.frame.size;

    NSButton* decodeMono = [[NSButton alloc]
        initWithFrame:NSMakeRect(10, windowSize.height - 50, 90, 40)];
    [decodeMono setTitle:@"DecodeMono"];
    [decodeMono setButtonType:NSMomentaryPushInButton];
    [decodeMono setTarget:self];
    [decodeMono setAction:@selector(doDecodeMono)];
    [self.view addSubview:decodeMono];

    NSButton* resample = [[NSButton alloc]
        initWithFrame:NSMakeRect(10, windowSize.height - 100, 90, 40)];
    [resample setTitle:@"Resample"];
    [resample setTarget:self];
    [resample setAction:@selector(doResample)];
    [self.view addSubview:resample];

    NSButton* decodeAny = [[NSButton alloc]
        initWithFrame:NSMakeRect(10, windowSize.height - 150, 90, 40)];
    [decodeAny setTitle:@"DecodeAny"];
    [decodeAny setTarget:self];
    [decodeAny setAction:@selector(doDecodeAny)];
    [self.view addSubview:decodeAny];

    NSButton* mix = [[NSButton alloc]
        initWithFrame:NSMakeRect(10, windowSize.height - 200, 90, 40)];
    [mix setTitle:@"Mix"];
    [mix setTarget:self];
    [mix setAction:@selector(doMix)];
    [self.view addSubview:mix];

    NSButton* recordAndMix = [[NSButton alloc]
        initWithFrame:NSMakeRect(10, windowSize.height - 250, 90, 40)];
    [recordAndMix setTitle:@"RecordAndMix"];
    [recordAndMix setTarget:self];
    [recordAndMix setAction:@selector(doRecordAndMix)];
    [self.view addSubview:recordAndMix];

    NSButton* stop = [[NSButton alloc]
        initWithFrame:NSMakeRect(110, windowSize.height - 50, 90, 40)];
    [stop setTitle:@"Stop"];
    [stop setTarget:self];
    [stop setAction:@selector(doStopTest)];
    [self.view addSubview:stop];

    _controller = [[AudioMixerTestController alloc] init];
}

- (void)doDecodeMono {
    [_controller doDecodeMono];
}

- (void)doResample {
    [_controller doResample];
}

- (void)doDecodeAny {
    [_controller doDecodeAny];
}

- (void)doMix {
    [_controller doMix];
}

- (void)doRecordAndMix {
    [_controller doRecordAndMix];
}

- (void)doStopTest {
    [_controller doStopTest];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end

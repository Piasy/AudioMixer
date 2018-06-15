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
    mix.frame = CGRectMake((screen.size.width - 200) / 2, 230, 200, 40);

    [mix addTarget:self
                  action:@selector(doMix)
        forControlEvents:UIControlEventTouchUpInside];

    [self.view addSubview:mix];

    UIButton* recordAndMix = [UIButton buttonWithType:UIButtonTypeSystem];
    [recordAndMix setTitle:@"RecordAndMix" forState:UIControlStateNormal];
    recordAndMix.titleLabel.font = [UIFont systemFontOfSize:20.0f];
    recordAndMix.frame =
        CGRectMake((screen.size.width - 200) / 2, 280, 200, 40);

    [recordAndMix addTarget:self
                     action:@selector(doRecordAndMix)
           forControlEvents:UIControlEventTouchUpInside];

    [self.view addSubview:recordAndMix];

    UIButton* stop = [UIButton buttonWithType:UIButtonTypeSystem];
    [stop setTitle:@"Stop" forState:UIControlStateNormal];
    stop.titleLabel.font = [UIFont systemFontOfSize:20.0f];
    stop.frame = CGRectMake((screen.size.width - 200) / 2, 380, 200, 40);

    [stop addTarget:self
                  action:@selector(doStopTest)
        forControlEvents:UIControlEventTouchUpInside];

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

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end

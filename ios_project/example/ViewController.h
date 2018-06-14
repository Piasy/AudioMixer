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


#import <UIKit/UIKit.h>
#import <AudioUnit/AudioUnit.h>

@interface ViewController : UIViewController

- (int32_t)RecordedDataIsAvailable:(const void*)audioSamples
                           nSamples:(size_t)nSamples
                    nBytesPerSample:(size_t)nBytesPerSample
                          nChannels:(size_t)nChannels
                      samplesPerSec:(uint32_t)samplesPerSec
                       totalDelayMS:(uint32_t)totalDelayMS
                         clockDrift:(int32_t)clockDrift
                    currentMicLevel:(uint32_t)currentMicLevel
                         keyPressed:(bool)keyPressed;

- (size_t)NeedMorePlayData:(size_t)nSamples
            nBytesPerSample:(size_t)nBytesPerSample
                  nChannels:(size_t)nChannels
              samplesPerSec:(uint32_t)samplesPerSec
               audioSamples:(void*)audioSamples;

@end


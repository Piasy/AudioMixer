#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/samplefmt.h>

#ifdef __cplusplus
}
#endif

namespace audio_mixer {

extern int32_t g_ssrc;
extern const AVSampleFormat kOutputSampleFormat;

}

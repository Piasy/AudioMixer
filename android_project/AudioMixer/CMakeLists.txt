cmake_minimum_required(VERSION 3.4.1)

set(CWD ${CMAKE_CURRENT_LIST_DIR})


include_directories(
    ../../libs/webrtc/include
    ../../libs/webrtc/include/third_party/abseil-cpp
    ../../libs/ffmpeg/include
    ../../djinni/support-lib/
    ../../djinni/support-lib/jni/
    ../../generated_src/cpp/
    ../../generated_src/jni/
    ../../src/
)


add_definitions(
    -DWEBRTC_POSIX
    -DWEBRTC_ANDROID
)


file(GLOB audio_mixer_sources
    src/main/jni/*.cpp
    ../../src/*.cpp
    ../../djinni/support-lib/jni/*.cpp
    ../../generated_src/jni/*.cpp
    ../../generated_src/cpp/*.cpp
)

file(GLOB audio_mixer_deps
    ../../libs/ffmpeg/lib/Android/${ANDROID_ABI}/libswresample.a
    ../../libs/ffmpeg/lib/Android/${ANDROID_ABI}/libavformat.a
    ../../libs/ffmpeg/lib/Android/${ANDROID_ABI}/libavcodec.a
    ../../libs/ffmpeg/lib/Android/${ANDROID_ABI}/libavutil.a
    ../../libs/webrtc/lib/Android/${ANDROID_ABI}/libwebrtc.a
)

add_library(audio_mixer SHARED
    ${audio_mixer_sources}
)

target_link_libraries(
    audio_mixer
    android
    log
    z
    ${audio_mixer_deps}
)

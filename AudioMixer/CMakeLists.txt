cmake_minimum_required(VERSION 3.4.1)

set(CWD ${CMAKE_CURRENT_LIST_DIR})


include_directories(${CWD}/libs/webrtc/include)
include_directories(${CWD}/libs/ffmpeg/include)


add_definitions(-DWEBRTC_POSIX)

add_library(audio_mixer SHARED
            src/main/cpp/audio_mixer_jni.cpp
            src/main/cpp/audio_mixer.cpp
            src/main/cpp/file_audio_source.cpp
            src/main/cpp/audio_resampler.cpp
            )

target_link_libraries(audio_mixer
                      android
                      log

                      ${CWD}/libs/webrtc/lib/${ANDROID_ABI}/libwebrtc.a
                      ${CWD}/src/main/jniLibs/${ANDROID_ABI}/libswresample.so
                      ${CWD}/src/main/jniLibs/${ANDROID_ABI}/libavutil.so
                      )

cmake_minimum_required(VERSION 3.4.1)

set(CWD ${CMAKE_CURRENT_LIST_DIR})


include_directories(${CWD}/libs/webrtc/include)


add_definitions(-DWEBRTC_POSIX)

add_library(audio_mixer SHARED
            src/main/cpp/audio_mixer_jni.cpp
            src/main/cpp/audio_mixer.cpp
            src/main/cpp/file_audio_source.cpp
            )

# Include libraries needed for hack_webrtc lib
target_link_libraries(audio_mixer
                      android
                      log

                      ${CWD}/libs/webrtc/lib/armeabi-v7a/libwebrtc.a
                      )

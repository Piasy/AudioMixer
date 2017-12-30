cmake_minimum_required(VERSION 3.4.1)

set(CWD ${CMAKE_CURRENT_LIST_DIR})


include_directories(${CWD}/../../libs/webrtc/include)
include_directories(${CWD}/../../libs/ffmpeg/include)


add_definitions(-DWEBRTC_POSIX)


file(GLOB audio_mixer_sources
    ${CWD}/../../src/*.cpp
)

file(GLOB ffmpeg_libs
    ${CWD}/../../libs/jniLibs/${ANDROID_ABI}/*.so
)

add_library(audio_mixer SHARED
            ${audio_mixer_sources}
            )

target_link_libraries(audio_mixer
                      android
                      log

                      ${CWD}/../../libs/webrtc/lib/${ANDROID_ABI}/libwebrtc.a
                      ${ffmpeg_libs}
                      )

#!/bin/bash
export IS_PIASY_LIB_RELEASE=true
./gradlew clean --stacktrace && \
./gradlew :android_project:AudioMixer:assembleRelease && \
./gradlew publishReleasePublicationToSonatypeRepository && \
./gradlew closeAndReleaseRepository
unset IS_PIASY_LIB_RELEASE

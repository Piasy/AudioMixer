#!/bin/bash
export IS_PIASY_LIB_RELEASE=true
./gradlew clean bintrayUpload --stacktrace
unset IS_PIASY_LIB_RELEASE

#!/bin/bash

base_dir=$(cd "`dirname "0"`" && pwd)
cpp_out="$base_dir/generated_src/cpp"
jni_out="$base_dir/generated_src/jni"
objc_out="$base_dir/generated_src/objc"
java_out="$base_dir/generated_src/java/com/github/piasy/audio_mixer"
java_package="com.github.piasy.audio_mixer"
namespace="audio_mixer"
objc_prefix="PAM"
djinni_file="$base_dir/djinni/audio_mixer.djinni"

rm -rf $base_dir/generated_src && \
/usr/local/djinni/src/run \
   --java-out $java_out \
   --java-package $java_package \
   --ident-java-field mFooBar \
   \
   --cpp-out $cpp_out \
   --cpp-namespace $namespace \
   \
   --jni-out $jni_out \
   --ident-jni-class NativeFooBar \
   --ident-jni-file NativeFooBar \
   \
   --objc-out $objc_out \
   --objc-type-prefix $objc_prefix \
   \
   --objcpp-out $objc_out \
   \
   --idl $djinni_file

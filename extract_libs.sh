#!/bin/bash

TAG=libs-20180610

wget https://github.com/Piasy/AudioMixer/releases/download/${TAG}/${TAG}.zip && \
rm -rf libs && \
unzip ${TAG}.zip

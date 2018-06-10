#!/bin/bash

find libs -name ".DS_Store" | xargs rm && \
zip -r libs-20180610.zip libs

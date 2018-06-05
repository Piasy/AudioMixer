#!/bin/bash

find libs -name ".DS_Store" | xargs rm && \
zip -r libs-20180605.zip libs

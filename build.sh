#!/usr/bin/env bash

docker build --platform linux/amd64 -t milestone3 .
docker run --platform linux/amd64 --rm -v $(pwd):/milestone3 -w /milestone3 milestone3

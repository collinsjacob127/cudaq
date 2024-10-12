#!/usr/bin/bash

docker pull nvcr.io/nvidia/nightly/cuda-quantum:latest
docker run -it --gpus all --name cuda-quantum nvcr.io/nvidia/nightly/cuda-quantum:latest

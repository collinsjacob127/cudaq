#!/usr/bin/bash
# This will run the docker if it's already downloaded.
# docker pull nvcr.io/nvidia/nightly/cuda-quantum:latest
docker run -it --name cuda-quantum nvcr.io/nvidia/nightly/cuda-quantum:latest

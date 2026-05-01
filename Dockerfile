FROM --platform=linux/amd64 ubuntu:24.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

ENV LD_LIBRARY_PATH=/milestone3/lib:$LD_LIBRARY_PATH

CMD make && ./milestone3

# =============================================================================
# Builder
# =============================================================================
FROM ubuntu:24.04 AS build

# Do not allow apt to be interactive
ENV DEBIAN_FRONTEND=noninteractive

# Update everything, install dependencies, and create build/protobufs directory.
RUN apt update -yy && \
    apt upgrade -yy && \
    apt install -yy build-essential cmake libssl-dev libtls-dev libgrpc++-dev \
                    protobuf-compiler-grpc && \
    mkdir -p /opt/chlorobot/build && \
    mkdir -p /opt/chlorobot/protos

# Change to the source directory and copy source files
WORKDIR /opt/chlorobot
COPY src/ ./src/
COPY include/ ./include/
COPY CMakeLists.txt .
COPY chlorobot_rpc.proto .

# We are ready to build
WORKDIR /opt/chlorobot/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)
# =============================================================================
# Runner
# =============================================================================
FROM ubuntu:24.04

# Do not allow apt to be interactive
ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt update -yy && \
    apt upgrade -yy && \
    apt install -yy ca-certificates libssl3t64 libtls28t64 libgrpc++1.51t64

# Add user
RUN groupadd chlorobot && useradd -m -g chlorobot chlorobot
USER chlorobot
RUN mkdir /home/chlorobot/bin/
COPY --chown=chlorobot:chlorobot --from=build \
    /opt/chlorobot/build/chlorobot /home/chlorobot/bin
ENTRYPOINT [ "/home/chlorobot/bin/chlorobot" ]

# Use latest Debian stable as base
FROM debian:stable-slim

# Update packages and install your custom packages
RUN apt-get update && \
    apt-get install -y \
    unzip \
    libx11-dev \
    libxft-dev \
    libasound2-dev \
    libxcursor-dev \
    git gcc g++ cmake wget

# Build TFSim deps and tfsim itself
WORKDIR /opt/TFSim
ADD src ./src/
ADD obj ./obj/
COPY  Makefile get_dep.sh .
RUN sh get_dep.sh
RUN make

# Add TFsim to PATH
ENV PATH="/opt/TFSim/:$PATH"

# Default command
CMD ["tfsim"]

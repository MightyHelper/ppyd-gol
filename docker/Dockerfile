# Stage 1: Build the project
FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    libopenmpi-dev \
    openmpi-bin \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the project files
COPY src/ src/
# COPY data/ data/
COPY CMakeLists.txt .

# Create build directory
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Stage 2: Runtime environment
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && \
    apt-get install -y \
    openmpi-bin \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the binaries from the build stage
COPY --from=builder /app/build/src/main/par /app/build/src/main/par
COPY --from=builder /app/build/src/main/par_gol /app/build/src/main/par_gol

# Copy other necessary directories
COPY data/ data/

# Note: Modify the default command as needed, such as specifying the number of processes and the binary to run.


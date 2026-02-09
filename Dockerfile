# --- Stage 1: Build (Cross-Compile) ---
# Use the official Toradex cross-toolchain for ARM64
FROM torizon/cross-toolchain-arm64:4 AS build

# Enable the architecture we are targeting (arm64) for package installation
# The base image is x86_64 but configured for cross-compilation
RUN sudo dpkg --add-architecture arm64 && \
    sudo apt-get update && \
    sudo apt-get install -y \
    libsdl2-dev:arm64 \
    build-essential \
    cmake \
    && sudo apt-get clean && sudo apt-get autoremove

# Set up the build directory
WORKDIR /app
COPY . .

# Build the application
# We use the cross-compilers provided by the environment (aarch64-linux-gnu-g++)
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DUSE_GLES2=ON .. && \
    make

# --- Stage 2: Runtime (Target Device) ---
# Use the base Wayland image optimized for iMX8 (includes Vivante GPU user-space libs)
FROM torizon/wayland-base-imx8:4 AS deploy

USER root

# Install the runtime version of SDL2
RUN apt-get update && apt-get install -y \
    libsdl2-2.0-0 \
	libdecor-0-0 \
    && apt-get clean && apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

# Copy the compiled binary from the build stage
COPY --from=build /app/build/SDL2Benchmark /usr/local/bin/SDL2Benchmark

USER torizon

# Set the environment to use the Wayland backend for SDL
ENV SDL_VIDEODRIVER=wayland
ENV SDL_WAYLAND_LIBDECOR=1

# The command to run
CMD ["SDL2Benchmark"]

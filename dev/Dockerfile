FROM ubuntu:22.04
# Dependencies for glvnd and X11.
RUN apt-get update \
  && apt-get install -y -qq --no-install-recommends \
    libglvnd0 \
    libgl1 \
    libglx0 \
    libegl1 \
    libxext6 \
    libx11-6 
# Dependencies for ek640 gui
RUN apt-get install -y \
    qtbase5-dev \
    libopencv-dev \
    libpcl-dev \
    cmake 
# Env vars for the nvidia-container-runtime.
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES graphics,utility,compute

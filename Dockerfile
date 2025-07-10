FROM devkitpro/devkita64:latest

# Install system tools and bash
RUN apt-get update && apt-get install -y \
    bash \
    cmake \
    make \
    git \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# (Optional) Install Switch libraries if needed for your build
# RUN dkp-pacman -Sy --noconfirm && \
#     dkp-pacman -S --noconfirm switch-sdl2 switch-mesa switch-glm switch-sdl2_ttf switch-sdl2_image

# Keep the container alive for CI
CMD ["tail", "-f", "/dev/null"] 
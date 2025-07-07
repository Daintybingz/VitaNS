FROM devkitpro/devkita64:latest

# Install system tools
RUN apt-get update && apt-get install -y cmake make git

# Install Switch libraries
RUN dkp-pacman -Sy --noconfirm && \
    dkp-pacman -S --noconfirm switch-sdl2 switch-mesa switch-glm switch-sdl2_ttf switch-sdl2_image

# Clean up (optional)
RUN apt-get clean && rm -rf /var/lib/apt/lists/* 
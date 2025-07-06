FROM devkitpro/devkita64:latest

# Install additional build tools
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    git \
    python3 \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /workspace

# Set environment variables for devkitPro
ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM
ENV DEVKITA64=/opt/devkitpro/devkitA64
ENV PATH=/opt/devkitpro/devkitA64/bin:/opt/devkitpro/tools/bin:$PATH

# Copy the project into the container
COPY . /workspace

# Default command: build the project
CMD ["/bin/bash"]

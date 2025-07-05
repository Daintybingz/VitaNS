FROM devkitpro/devkita64:latest

# Install additional build tools
RUN apt-get update && apt-get install -y cmake make git python3 g++

# Set up working directory
WORKDIR /workspace

# Copy the project into the container
COPY . /workspace

# Set environment variables for devkitPro
ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM
ENV DEVKITA64=/opt/devkitpro/devkitA64
ENV PATH=/Users/Gbins/.codeium/windsurf/bin:/opt/homebrew/Cellar/pyenv-virtualenv/1.2.4/shims:/Users/Gbins/.pyenv/shims:/Users/Gbins/.pyenv/bin:/opt/homebrew/Cellar/pyenv-virtualenv/1.2.4/shims:/opt/devkitpro/devkitA64/bin:/opt/devkitpro/devkitA64/bin:/Users/Gbins/.codeium/windsurf/bin:/opt/homebrew/opt/llvm/bin:/Users/Gbins/.codeium/windsurf/bin:/Applications/Wine Stable.app/Contents/Resources/wine/bin:/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/llvm/bin:/opt/homebrew/bin:/opt/homebrew/sbin:/usr/local/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/local/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/appleinternal/bin://Applications/Topaz Gigapixel AI.app/Contents/Resources/bin:/Library/Apple/usr/bin:/opt/devkitpro/tools/bin:/usr/local/share/dotnet:~/.dotnet/tools:/Users/Gbins/.codeium/windsurf/bin:/opt/homebrew/Cellar/pyenv-virtualenv/1.2.4/shims:/Users/Gbins/.pyenv/bin:/opt/devkitpro/devkitA64/bin:/opt/homebrew/opt/llvm/bin:/Applications/Wine Stable.app/Contents/Resources/wine/bin:/Users/Gbins/.local/bin:/Users/Gbins/PS2EMU/PCSXBOX/nxdk/bin:/usr/local/opt/llvm/bin:/Users/Gbins/.local/bin:/Users/Gbins/PS2EMU/PCSXBOX/nxdk/bin:/usr/local/opt/llvm/bin:/Users/Gbins/.local/bin:/opt/devkitpro/devkitA64/bin:/opt/devkitpro/tools/bin

# Default command: build the project
CMD ["/bin/bash"]

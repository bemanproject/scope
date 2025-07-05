FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

USER vscode

# Install latest cmake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN sudo apt-get update && sudo apt-get install -y cmake

# Install pre-commit and ninja
RUN sudo apt-get install -y pipx && pipx install pre-commit && pipx install ninja

# Newer gcc/ llvm is needed to avoid ASAN Stalling
# See: https://github.com/google/sanitizers/issues/1614
# Minimal version: clang-18.1.3, gcc-13.2
RUN sudo apt-get install -y gcc-14

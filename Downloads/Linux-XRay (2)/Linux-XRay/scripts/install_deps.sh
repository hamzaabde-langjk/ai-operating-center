#!/bin/bash
set -e

echo "Installing Linux X-Ray Vision dependencies..."
echo "=============================================="

# Detect distribution
if [ -f /etc/arch-release ]; then
    DISTRO="arch"
elif [ -f /etc/debian_version ]; then
    DISTRO="debian"
elif [ -f /etc/fedora-release ]; then
    DISTRO="fedora"
elif [ -f /etc/SuSE-release ] || [ -f /etc/SUSE-brand ]; then
    DISTRO="suse"
else
    echo "Unsupported distribution"
    exit 1
fi

echo "Detected distribution: $DISTRO"

case $DISTRO in
    arch)
        sudo pacman -S --needed \
            base-devel cmake clang llvm \
            qt6-base qt6-tools \
            spdlog sqlite \
            gtest benchmark \
            libbpf linux-headers \
            glm mesa \
            python python-pip
        ;;
    debian)
        sudo apt-get update
        sudo apt-get install -y \
            build-essential cmake clang llvm \
            qt6-base-dev qt6-tools-dev \
            libspdlog-dev libsqlite3-dev \
            libgtest-dev libbenchmark-dev \
            libbpf-dev linux-headers-$(uname -r) \
            libglm-dev libgl1-mesa-dev \
            python3 python3-pip
        ;;
    fedora)
        sudo dnf install -y \
            gcc-c++ cmake clang llvm \
            qt6-qtbase-devel qt6-qttools-devel \
            spdlog-devel sqlite-devel \
            gtest-devel benchmark-devel \
            libbpf-devel kernel-headers \
            glm-devel mesa-libGL-devel \
            python3 python3-pip
        ;;
    suse)
        sudo zypper install -y \
            gcc-c++ cmake clang llvm \
            qt6-base-devel qt6-tools-devel \
            spdlog-devel sqlite3-devel \
            gtest-devel google-benchmark-devel \
            libbpf-devel kernel-devel \
            glm-devel Mesa-libGL-devel \
            python3 python3-pip
        ;;
esac

# Install Python dependencies
pip3 install --user numpy scipy scikit-learn pandas matplotlib

echo "Dependencies installed!"

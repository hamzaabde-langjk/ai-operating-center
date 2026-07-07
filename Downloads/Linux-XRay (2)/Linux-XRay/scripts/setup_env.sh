#!/bin/bash

# Linux X-Ray Vision Environment Setup

export XRAY_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export XRAY_BUILD_DIR="$XRAY_ROOT/build"
export XRAY_DATA_DIR="$XRAY_ROOT/data"
export PATH="$XRAY_BUILD_DIR/cli:$PATH"

# eBPF requirements
export BPFTOOL="$(which bpftool 2>/dev/null || echo '/usr/sbin/bpftool')"

# Python AI path
export PYTHONPATH="$XRAY_ROOT/ai:$PYTHONPATH"

echo "Linux X-Ray Vision environment configured"
echo "XRAY_ROOT: $XRAY_ROOT"
echo "Add 'source $XRAY_ROOT/scripts/setup_env.sh' to your .bashrc"

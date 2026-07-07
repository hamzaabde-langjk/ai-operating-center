# Linux X-Ray Vision

A professional open-source Linux system monitor with real-time eBPF tracing and immersive 3D visualization.

## Features

- **Real-time eBPF tracing**: Kernel-level monitoring of processes, files, network, and more
- **Interactive 3D visualization**: Explore your system as a dynamic universe
- **Security analysis**: Detect anomalies, privilege escalation, and threats
- **AI-powered insights**: Natural language explanations and anomaly detection
- **Timeline & Replay**: Record and replay system activity
- **Plugin SDK**: Extend functionality with custom plugins
- **Multiple interfaces**: GUI (Qt6), CLI tools, and programmatic API

## Quick Start

### Dependencies

```bash
# Install all dependencies
./scripts/install_deps.sh
```

### Build

```bash
./scripts/build.sh
```

### Run

```bash
# GUI mode
./build/gui/xray-gui

# CLI mode
./build/cli/xray --monitor

# Top-like process view
./build/cli/xray-top

# System monitor
./build/cli/xray-monitor
```

## Architecture

```
+-------------+  +-------------+  +-------------+
|   GUI (Qt6) |  |  CLI Tools  |  |   Plugins   |
+------+------+  +------+------+  +------+------+
       |                |                |
       +----------------+----------------+
                          |
              +-----------+-----------+
              |     Core Engine       |
              |  (Event Bus, Graph)   |
              +-----------+-----------+
                          |
       +------------------+------------------+
       |                  |                  |
+------+------+  +--------+--------+  +-----+------+
|  eBPF Kernel|  |  3D Renderer    |  |  Database  |
|  Probes     |  |  (OpenGL)       |  |  (SQLite)  |
+-------------+  +-----------------+  +------------+
```

## Project Structure

- `kernel/` - eBPF programs (process, file, network trackers)
- `engine/` - Core event system (bus, dispatcher, filters, cache)
- `graph/` - Dynamic graph engine (nodes, edges, layouts)
- `renderer/` - OpenGL 3D renderer (shaders, camera, scene)
- `gui/` - Qt6 user interface
- `database/` - SQLite storage layer
- `security/` - Threat detection and analysis
- `ai/` - Python AI/ML components
- `cli/` - Command-line tools
- `plugins/` - Plugin SDK and manager
- `tests/` - Unit and integration tests

## License

MIT License - See LICENSE file for details.

## Contributing

See CONTRIBUTING.md for guidelines.

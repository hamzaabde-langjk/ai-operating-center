# Architecture Documentation

## Component Overview

### 1. Kernel Layer (eBPF)

The kernel layer uses eBPF programs attached to tracepoints and kprobes to capture system events with minimal overhead.

**Programs:**
- `process_tracker.bpf.c` - Process lifecycle (exec, fork, exit)
- `file_tracker.bpf.c` - File operations (open, read, write, unlink)
- `network_tracker.bpf.c` - Network activity (TCP connect, UDP send/recv)

**Communication:** Ring buffers for zero-copy event delivery to userspace.

### 2. Event Engine

Thread-safe, lock-free event processing pipeline:

```
eBPF Ring Buffer -> Event Bus -> Filters -> Transformers -> Dispatch
                              |
                         Subscribers (GUI, DB, Security, AI)
```

**Key Classes:**
- `EventBus` - Thread-safe pub/sub with configurable queue limits
- `EventDispatcher` - Filter and transform events before dispatch
- `EventFilter` - Composable predicates (AND, OR, NOT)
- `TimestampEngine` - Time-indexed event storage for replay
- `Serializer` - JSON, binary, and compressed formats
- `LRUCache` - Template-based LRU cache for hot data

### 3. Graph Engine

Dynamic graph representation of system state:

**Nodes:** Processes, Files, Sockets, Containers, Users, etc.
**Edges:** Parent-child, file operations, network connections, dependencies.

**Layout Algorithms:**
- Force-directed (default)
- Spiral galaxy (for directory structures)
- Hierarchical (for process trees)

### 4. 3D Renderer

OpenGL-based visualization with Vulkan-ready architecture:

**Features:**
- Instanced rendering for thousands of nodes
- Particle systems for system calls
- Glowing effects for active elements
- Interactive camera (orbit, pan, zoom)
- Ray-based object picking

### 5. Database Layer

SQLite with WAL mode for concurrent reads/writes:

**Tables:**
- `events` - All system events with indices
- `snapshots` - System state captures
- `replay_sessions` - Recorded event streams
- `bookmarks` - User annotations
- `statistics` - Time-series metrics
- `ai_analysis` - ML-generated insights

### 6. Security Layer

Multi-layered threat detection:

**Built-in Detectors:**
- Privilege escalation (setuid, capabilities)
- Suspicious processes (known signatures)
- Code injection (ptrace, mmap patterns)
- Persistence mechanisms (cron, systemd)
- Network anomalies (C2 ports, data exfiltration)
- Container escapes

**Custom Rules:** User-defined predicates with severity levels.

### 7. AI Engine (Python)

- Anomaly detection using statistical methods
- Behavior analysis with clustering
- Pattern recognition for attack signatures
- Ollama integration for natural language explanations
- Resource prediction with time-series forecasting

## Threading Model

```
Main Thread:     GUI updates, user input
Event Thread:    EventBus worker (dispatch)
eBPF Thread:     Ring buffer polling
DB Thread:       Async database writes
AI Thread:       Python model inference
Render Thread:   OpenGL rendering (Qt6)
```

## Performance Targets

- Event throughput: >100,000 events/second
- Graph nodes: >10,000 with 60 FPS
- Database: >10,000 inserts/second
- Memory: <500MB baseline
- CPU: <5% overhead for eBPF probes

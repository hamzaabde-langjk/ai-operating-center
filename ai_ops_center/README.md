# 🧠 AI Operating Center

### Autonomous Multi-Agent AI System powered by Ollama

[![Python 3.12+](https://img.shields.io/badge/python-3.12+-blue.svg)](https://www.python.org/downloads/)
[![Flask](https://img.shields.io/badge/flask-3.0.0-green.svg)](https://flask.palletsprojects.com/)
[![Ollama](https://img.shields.io/badge/ollama-0.1.0-orange.svg)](https://ollama.ai/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## 📖 Overview

**AI Operating Center** is a production-ready, autonomous multi-agent AI system that runs 24/7. Simply provide a goal, and the system handles everything - planning, task creation, agent assignment, execution, monitoring, and failure recovery.

> 🎯 **One Goal. Infinite Possibilities. Zero Manual Intervention.**

---

## ✨ Key Features

### 🎯 Autonomous Operation
- **Goal-Driven Execution**: Define a goal, and watch the AI system orchestrate itself
- **24/7 Operation**: Runs continuously with automatic recovery and state persistence
- **Zero Human Intervention**: Full end-to-end automation
- **Self-Healing**: Automatically detects and fixes failures

### 🤖 Multi-Agent System
- **20+ Specialized Agents**:
  - CEO Agent - Strategic oversight and coordination
  - Planner Agent - Task decomposition and planning
  - Architect Agent - System architecture design
  - Research Agent - Information gathering and analysis
  - Coding Agent - Code implementation
  - Review Agent - Code review and quality assurance
  - Testing Agent - Test creation and execution
  - Debug Agent - Bug identification and fixing
  - Security Agent - Security analysis and auditing
  - Performance Agent - Performance optimization
  - Documentation Agent - Documentation generation
  - Git Agent - Version control operations
  - Memory Agent - Memory management
  - Learning Agent - Continuous learning and improvement
  - Monitor Agent - System monitoring
  - Notification Agent - Alerting and notifications
  - Scheduler Agent - Task scheduling
  - Plugin Manager - Plugin management
  - Agent Factory - Agent creation and management
  - Linux Agent - Linux system operations
  - Terminal Agent - Terminal operations

### 🧠 Intelligent Memory System
- **Short-Term Memory**: Recent context and interactions
- **Long-Term Memory**: Persistent knowledge storage
- **Shared Memory**: Cross-agent information sharing
- **Project Memory**: Project-specific knowledge
- **Vector Search**: Semantic similarity search

### 🎨 Modern Web Dashboard
- **Real-Time Updates**: Flask-SocketIO for instant updates
- **Live Monitoring**:
  - CPU, RAM, Disk, GPU usage
  - Network statistics
  - Temperature monitoring
  - Process monitoring
  - Docker container monitoring
- **Agent Visualization**: Interactive Cytoscape.js graph
- **Embedded Terminal**: Full xterm.js terminal
- **File Manager**: Complete file system management
- **Project Management**: Projects, tasks, milestones

### 🔌 Plugin System
- **Auto-Discovery**: Automatically detects new agents
- **Easy Extension**: Add agents by creating a new folder
- **Hot-Reload**: Plugin changes detected instantly

### 🛡️ Security & Authentication
- **User Authentication**: Secure login system
- **Role-Based Permissions**: Granular access control
- **CSRF Protection**: Cross-site request forgery protection
- **Rate Limiting**: API rate limiting
- **Input Validation**: All inputs validated

### 📊 Monitoring & Logging
- **Comprehensive Logging**: All agent activities logged
- **System Metrics**: Real-time resource monitoring
- **Error Tracking**: Automatic error detection and reporting
- **Performance Monitoring**: Agent and task performance metrics

---

## 🛠️ Technology Stack

### Backend
- **Python 3.12+** - Core programming language
- **Flask** - Web framework
- **Flask-SocketIO** - WebSocket support
- **SQLAlchemy** - ORM for database operations
- **SQLite** - Database
- **AsyncIO** - Asynchronous operations
- **APScheduler** - Task scheduling
- **psutil** - System monitoring
- **watchdog** - File system monitoring

### Frontend
- **HTML5** - Structure
- **Jinja2** - Templating engine
- **Bootstrap 5** - UI framework
- **JavaScript** - Client-side logic
- **Chart.js** - Charts and graphs
- **Cytoscape.js** - Network visualization
- **xterm.js** - Terminal emulator

### AI/ML
- **Ollama** - Local LLM serving
- **Multiple Models**: Support for qwen3, deepseek-coder, llama3, and more

---

## 📋 Prerequisites

- **Python 3.12 or higher**
- **Ollama** installed and running
- **Docker** (optional, for containerized deployment)
- **Git** (for version control)

---

## 🚀 Installation

### Option 1: Local Installation

1. **Clone the repository**
```bash
git clone https://github.com//ai-operating-center.git
cd ai-operating-center


# 2. Install dependencies
pip install -r requirements.txt

# 3. Make sure Ollama is running locally
ollama serve

# 4. Pull recommended models
ollama pull qwen3
ollama pull llama3
ollama pull deepseek-coder

# 5. Run the application
chmod +x start.sh
./start.sh


# 6. Open in browser 
# http://....
url of cloudaflare 
# Login: admin / admin
#injoy 

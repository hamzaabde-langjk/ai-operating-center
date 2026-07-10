"""Configuration module for AI Operations Center."""
import os
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent

class Config:
    """Base configuration."""
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'ai-ops-center-dev-secret-key-2026'
    SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL') or f'sqlite:///{BASE_DIR}/database/ai_ops.db'
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    SQLALCHEMY_ENGINE_OPTIONS = {
        'pool_pre_ping': True,
        'pool_recycle': 300,
    }

    # Ollama
    OLLAMA_HOST = os.environ.get('OLLAMA_HOST') or 'http://localhost:11434'
    OLLAMA_TIMEOUT = int(os.environ.get('OLLAMA_TIMEOUT') or 120)

    # Default models per agent
    DEFAULT_MODELS = {
        'ceo': 'qwen3',
        'planner': 'qwen3',
        'architect': 'qwen3',
        'research': 'llama3',
        'coding': 'deepseek-coder',
        'review': 'qwen3',
        'critic': 'qwen3',
        'testing': 'qwen3',
        'debug': 'deepseek-coder',
        'linux': 'qwen3',
        'terminal': 'qwen3',
        'security': 'qwen3',
        'performance': 'qwen3',
        'documentation': 'qwen3',
        'git': 'qwen3',
        'memory': 'qwen3',
        'learning': 'qwen3',
        'monitor': 'qwen3',
        'notification': 'qwen3',
        'scheduler': 'qwen3',
        'plugin_manager': 'qwen3',
        'agent_factory': 'qwen3',
    }

    # LLM parameters
    DEFAULT_TEMPERATURE = float(os.environ.get('DEFAULT_TEMPERATURE') or 0.7)
    DEFAULT_MAX_TOKENS = int(os.environ.get('DEFAULT_MAX_TOKENS') or 4096)
    DEFAULT_CONTEXT_LENGTH = int(os.environ.get('DEFAULT_CONTEXT_LENGTH') or 8192)

    # Memory
    SHORT_MEMORY_LIMIT = int(os.environ.get('SHORT_MEMORY_LIMIT') or 100)
    LONG_MEMORY_LIMIT = int(os.environ.get('LONG_MEMORY_LIMIT') or 1000)
    VECTOR_DIMENSION = int(os.environ.get('VECTOR_DIMENSION') or 768)

    # Agent limits
    MAX_AGENTS = int(os.environ.get('MAX_AGENTS') or 50)
    MAX_CONCURRENT_TASKS = int(os.environ.get('MAX_CONCURRENT_TASKS') or 20)
    TASK_TIMEOUT = int(os.environ.get('TASK_TIMEOUT') or 300)
    MAX_RETRIES = int(os.environ.get('MAX_RETRIES') or 3)

    # Monitoring
    MONITOR_INTERVAL = int(os.environ.get('MONITOR_INTERVAL') or 5)
    LOG_RETENTION_DAYS = int(os.environ.get('LOG_RETENTION_DAYS') or 30)

    # Paths
    LOGS_DIR = BASE_DIR / 'logs'
    PROJECTS_DIR = BASE_DIR / 'projects'
    UPLOADS_DIR = BASE_DIR / 'uploads'
    PLUGINS_DIR = BASE_DIR / 'plugins'

    # Security
    RATE_LIMIT_DEFAULT = os.environ.get('RATE_LIMIT_DEFAULT') or '200 per minute'
    SESSION_LIFETIME = int(os.environ.get('SESSION_LIFETIME') or 3600)

    # SocketIO
    SOCKETIO_ASYNC_MODE = 'eventlet'
    SOCKETIO_CORS_ALLOWED_ORIGINS = '*'

    # Scheduler
    SCHEDULER_API_ENABLED = True
    SCHEDULER_TIMEZONE = 'UTC'

class DevelopmentConfig(Config):
    DEBUG = True

class ProductionConfig(Config):
    DEBUG = False

config_map = {
    'development': DevelopmentConfig,
    'production': ProductionConfig,
    'default': DevelopmentConfig,
}

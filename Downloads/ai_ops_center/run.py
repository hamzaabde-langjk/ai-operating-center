"""Entry point to start the AI Operations Center."""
import os
import sys
import asyncio
import logging

from app import create_app, socketio
from core.ollama_client import OllamaClient
from core.agent_factory import AgentFactory
from services.task_service import TaskService
from services.project_service import ProjectService
from services.monitor_service import MonitorService
from services.terminal_service import TerminalService
from config import config_map

logger = logging.getLogger(__name__)

class AIOpsCenter:
    """Main application controller."""

    def __init__(self):
        self.app = None
        self.agent_factory = None
        self.task_service = None
        self.project_service = None
        self.monitor_service = None
        self.terminal_service = None
        self._running = False

    async def initialize(self):
        """Initialize all services."""
        logger.info("Initializing AI Operations Center...")

        # Create Flask app
        config_name = os.environ.get('FLASK_ENV', 'development')
        self.app = create_app(config_name)

        # Initialize Ollama client
        ollama = OllamaClient(
            host=self.app.config['OLLAMA_HOST'],
            timeout=self.app.config['OLLAMA_TIMEOUT'],
        )

        # Test Ollama connection
        try:
            models = await ollama.list_models()
            logger.info(f"Connected to Ollama. Available models: {len(models)}")
        except Exception as e:
            logger.warning(f"Ollama connection failed: {e}")

        # Initialize services
        self.agent_factory = AgentFactory(ollama, self.app.config['DEFAULT_MODELS'])
        self.task_service = TaskService()
        self.project_service = ProjectService()
        self.monitor_service = MonitorService(self.app.config['MONITOR_INTERVAL'])
        self.terminal_service = TerminalService()

        # Create all default agents
        self.agent_factory.create_all_default_agents()
        logger.info(f"Created {len(self.agent_factory.agents)} agents")

        # Start agent loops
        await self.agent_factory.start_all()

        # Start monitoring
        asyncio.create_task(self.monitor_service.start_monitoring())

        # Setup event forwarding
        self.agent_factory.on_event(self._on_agent_event)
        self.task_service.on_event(self._on_task_event)

        self._running = True
        logger.info("AI Operations Center initialized successfully")

    def _on_agent_event(self, event_type: str, data: dict):
        """Forward agent events to WebSocket clients."""
        try:
            socketio.emit(f'agent_{event_type}', data, namespace='/')
        except Exception:
            pass

    def _on_task_event(self, event_type: str, data: dict):
        """Forward task events to WebSocket clients."""
        try:
            socketio.emit(f'task_{event_type}', data, namespace='/')
        except Exception:
            pass

    def run(self):
        """Run the application."""
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self.initialize())

        host = os.environ.get('HOST', '0.0.0.0')
        port = int(os.environ.get('PORT', 5000))

        logger.info(f"Starting AI Operations Dashboard on http://{host}:{port}")
        socketio.run(
            self.app,
            host=host,
            port=port,
            debug=self.app.config.get('DEBUG', False),
            use_reloader=False,
        )

center = AIOpsCenter()

if __name__ == '__main__':
    center.run()

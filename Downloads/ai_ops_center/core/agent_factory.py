"""Factory for creating and managing all agents."""
import asyncio
import logging
from typing import Dict, List, Optional, Type, Any
from core.agent_base import AgentBase
from core.ollama_client import OllamaClient

# Import all agent implementations
from agents.ceo_agent import CEOAgent
from agents.planner_agent import PlannerAgent
from agents.architect_agent import ArchitectAgent
from agents.research_agent import ResearchAgent
from agents.coding_agent import CodingAgent
from agents.review_agent import ReviewAgent
from agents.critic_agent import CriticAgent
from agents.testing_agent import TestingAgent
from agents.debug_agent import DebugAgent
from agents.linux_agent import LinuxAgent
from agents.terminal_agent import TerminalAgent
from agents.security_agent import SecurityAgent
from agents.performance_agent import PerformanceAgent
from agents.documentation_agent import DocumentationAgent
from agents.git_agent import GitAgent
from agents.memory_agent import MemoryAgent
from agents.learning_agent import LearningAgent
from agents.monitor_agent import MonitorAgent
from agents.notification_agent import NotificationAgent
from agents.scheduler_agent import SchedulerAgent
from agents.plugin_manager import PluginManager
from agents.agent_factory_agent import AgentFactoryAgent

logger = logging.getLogger(__name__)

AGENT_REGISTRY: Dict[str, Type[AgentBase]] = {
    'ceo': CEOAgent,
    'planner': PlannerAgent,
    'architect': ArchitectAgent,
    'research': ResearchAgent,
    'coding': CodingAgent,
    'review': ReviewAgent,
    'critic': CriticAgent,
    'testing': TestingAgent,
    'debug': DebugAgent,
    'linux': LinuxAgent,
    'terminal': TerminalAgent,
    'security': SecurityAgent,
    'performance': PerformanceAgent,
    'documentation': DocumentationAgent,
    'git': GitAgent,
    'memory': MemoryAgent,
    'learning': LearningAgent,
    'monitor': MonitorAgent,
    'notification': NotificationAgent,
    'scheduler': SchedulerAgent,
    'plugin_manager': PluginManager,
    'agent_factory': AgentFactoryAgent,
}

class AgentFactory:
    """Factory that creates, manages, and orchestrates all agents."""

    def __init__(self, ollama_client: OllamaClient, default_models: Dict[str, str]):
        self.ollama = ollama_client
        self.default_models = default_models
        self.agents: Dict[str, AgentBase] = {}
        self._tasks: List[asyncio.Task] = []
        self._event_callbacks: List[Any] = []

    def create_agent(self, role: str, name: Optional[str] = None, model: Optional[str] = None) -> AgentBase:
        """Create an agent by role."""
        if role not in AGENT_REGISTRY:
            raise ValueError(f"Unknown agent role: {role}")

        agent_class = AGENT_REGISTRY[role]
        agent_name = name or f"{role.title()}_Agent_{len(self.agents)}"
        agent_model = model or self.default_models.get(role, 'qwen3')

        agent = agent_class(
            name=agent_name,
            role=role,
            model=agent_model,
            ollama_client=self.ollama,
        )

        agent.on_event(self._on_agent_event)
        self.agents[agent.id] = agent
        logger.info(f"Created agent: {agent_name} ({role}) with model {agent_model}")
        return agent

    def create_all_default_agents(self):
        """Create all default agents from the registry."""
        for role in AGENT_REGISTRY:
            self.create_agent(role)

    def get_agent(self, agent_id: str) -> Optional[AgentBase]:
        return self.agents.get(agent_id)

    def get_agent_by_name(self, name: str) -> Optional[AgentBase]:
        for agent in self.agents.values():
            if agent.name == name:
                return agent
        return None

    def get_agent_by_role(self, role: str) -> Optional[AgentBase]:
        for agent in self.agents.values():
            if agent.role == role:
                return agent
        return None

    def list_agents(self) -> List[Dict[str, Any]]:
        return [agent.to_dict() for agent in self.agents.values()]

    def _on_agent_event(self, event_type: str, data: Dict[str, Any]):
        """Forward agent events to all registered callbacks."""
        for cb in self._event_callbacks:
            try:
                cb(event_type, data)
            except Exception:
                pass

    def on_event(self, callback):
        self._event_callbacks.append(callback)

    async def start_all(self):
        """Start all agent run loops."""
        for agent in self.agents.values():
            task = asyncio.create_task(agent.run_loop())
            self._tasks.append(task)
        logger.info(f"Started {len(self._tasks)} agent loops")

    async def stop_all(self):
        """Stop all agents gracefully."""
        for agent in self.agents.values():
            agent.stop()
        for task in self._tasks:
            task.cancel()
            try:
                await task
            except asyncio.CancelledError:
                pass
        self._tasks.clear()
        logger.info("All agents stopped")

    async def assign_task(self, agent_id: str, task: Any) -> bool:
        """Assign a task to a specific agent."""
        agent = self.get_agent(agent_id)
        if not agent:
            return False
        return await agent.enqueue_task(task)

    async def broadcast_task(self, task: Any, role_filter: Optional[List[str]] = None) -> int:
        """Broadcast a task to all agents matching the role filter."""
        count = 0
        for agent in self.agents.values():
            if role_filter and agent.role not in role_filter:
                continue
            if await agent.enqueue_task(task):
                count += 1
        return count

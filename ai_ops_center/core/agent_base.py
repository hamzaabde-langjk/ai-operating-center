"""Base agent class for the multi-agent AI system."""
import asyncio
import json
import logging
import uuid
from abc import ABC, abstractmethod
from datetime import datetime
from typing import Any, Dict, List, Optional, Callable
from dataclasses import dataclass, field, asdict

from core.ollama_client import OllamaClient

logger = logging.getLogger(__name__)

@dataclass
class AgentTask:
    id: str
    title: str
    description: str = ''
    priority: int = 3  # 1=critical, 4=low
    status: str = 'pending'
    assigned_agent: str = ''
    logs: List[str] = field(default_factory=list)
    created_at: str = field(default_factory=lambda: datetime.utcnow().isoformat())
    finished_at: Optional[str] = None
    retry_count: int = 0
    dependencies: List[str] = field(default_factory=list)
    result: Any = None
    error: Optional[str] = None

class AgentBase(ABC):
    """Abstract base class for all AI agents."""

    def __init__(
        self,
        name: str,
        role: str,
        model: str,
        ollama_client: OllamaClient,
        system_prompt: str = '',
        temperature: float = 0.7,
        max_tokens: int = 4096,
    ):
        self.id = str(uuid.uuid4())
        self.name = name
        self.role = role
        self.model = model
        self.ollama = ollama_client
        self.system_prompt = system_prompt or self._default_system_prompt()
        self.temperature = temperature
        self.max_tokens = max_tokens

        self.status = 'idle'
        self.memory: List[Dict[str, Any]] = []
        self.queue: List[AgentTask] = []
        self.logs: List[str] = []
        self.task_history: List[str] = []
        self.statistics: Dict[str, Any] = {
            'tasks_completed': 0,
            'tasks_failed': 0,
            'total_tokens_used': 0,
            'avg_response_time': 0.0,
            'last_active': None,
        }

        self._current_task: Optional[AgentTask] = None
        self._lock = asyncio.Lock()
        self._running = False
        self._callbacks: List[Callable] = []

    def _default_system_prompt(self) -> str:
        return f"You are the {self.role} agent in an AI Operations Center. Be concise, accurate, and helpful."

    def log(self, message: str, level: str = 'INFO'):
        """Log a message with timestamp."""
        timestamp = datetime.utcnow().isoformat()
        entry = f"[{timestamp}] [{level}] {self.name}: {message}"
        self.logs.append(entry)
        logger.info(entry)
        self._notify('log', {'agent': self.name, 'message': entry})

    def _notify(self, event_type: str, data: Dict[str, Any]):
        """Notify all registered callbacks."""
        for cb in self._callbacks:
            try:
                cb(event_type, data)
            except Exception:
                pass

    def on_event(self, callback: Callable):
        """Register an event callback."""
        self._callbacks.append(callback)

    async def think(self, prompt: str, context: Optional[List[Dict]] = None) -> str:
        """Send a thought to the LLM and return the response."""
        messages = context or []
        messages.append({'role': 'user', 'content': prompt})

        start_time = datetime.utcnow()
        try:
            response = await self.ollama.chat(
                model=self.model,
                messages=messages,
                temperature=self.temperature,
                max_tokens=self.max_tokens,
                system=self.system_prompt,
            )
            elapsed = (datetime.utcnow() - start_time).total_seconds()

            content = response.get('content', '')
            usage = response.get('usage', {})
            tokens = usage.get('total_tokens', 0)

            self.statistics['total_tokens_used'] += tokens
            self._update_avg_response_time(elapsed)
            self.statistics['last_active'] = datetime.utcnow().isoformat()

            return content
        except Exception as e:
            self.log(f"Think error: {e}", 'ERROR')
            raise

    def _update_avg_response_time(self, elapsed: float):
        """Update rolling average response time."""
        n = self.statistics['tasks_completed'] + self.statistics['tasks_failed'] + 1
        old_avg = self.statistics['avg_response_time']
        self.statistics['avg_response_time'] = (old_avg * (n - 1) + elapsed) / n

    async def enqueue_task(self, task: AgentTask) -> bool:
        """Add a task to the agent's queue."""
        async with self._lock:
            self.queue.append(task)
            self.log(f"Task {task.id} enqueued: {task.title}")
            self._notify('task_enqueued', {'agent': self.name, 'task': asdict(task)})
            return True

    async def process_next(self) -> Optional[AgentTask]:
        """Process the next task in the queue."""
        async with self._lock:
            if not self.queue or self.status == 'busy':
                return None

            self._current_task = self.queue.pop(0)
            self.status = 'busy'
            self._current_task.status = 'running'
            self._current_task.started_at = datetime.utcnow().isoformat()

        task = self._current_task
        self.log(f"Starting task {task.id}: {task.title}")
        self._notify('task_started', {'agent': self.name, 'task': asdict(task)})

        try:
            result = await self.execute(task)
            task.result = result
            task.status = 'completed'
            task.finished_at = datetime.utcnow().isoformat()
            self.statistics['tasks_completed'] += 1
            self.log(f"Task {task.id} completed successfully")
            self._notify('task_completed', {'agent': self.name, 'task': asdict(task)})
        except Exception as e:
            task.error = str(e)
            task.retry_count += 1
            if task.retry_count <= 3:
                task.status = 'retrying'
                self.queue.insert(0, task)
                self.log(f"Task {task.id} failed, retrying ({task.retry_count}/3): {e}", 'WARNING')
            else:
                task.status = 'failed'
                task.finished_at = datetime.utcnow().isoformat()
                self.statistics['tasks_failed'] += 1
                self.log(f"Task {task.id} failed permanently: {e}", 'ERROR')
            self._notify('task_failed', {'agent': self.name, 'task': asdict(task)})
        finally:
            self.task_history.append(task.id)
            self._current_task = None
            self.status = 'idle'

        return task

    @abstractmethod
    async def execute(self, task: AgentTask) -> Any:
        """Execute the assigned task. Must be implemented by subclasses."""
        pass

    async def run_loop(self):
        """Continuously process tasks from the queue."""
        self._running = True
        self.log(f"{self.name} agent loop started")
        while self._running:
            try:
                await self.process_next()
            except Exception as e:
                self.log(f"Error in run loop: {e}", 'ERROR')
            await asyncio.sleep(0.5)

    def stop(self):
        """Stop the agent's run loop."""
        self._running = False
        self.status = 'offline'
        self.log(f"{self.name} agent stopped")

    def to_dict(self) -> Dict[str, Any]:
        """Serialize agent state to dictionary."""
        return {
            'id': self.id,
            'name': self.name,
            'role': self.role,
            'model': self.model,
            'status': self.status,
            'memory': self.memory[-50:],  # Last 50 entries
            'queue_length': len(self.queue),
            'logs': self.logs[-100:],  # Last 100 logs
            'task_history': self.task_history[-100:],
            'statistics': self.statistics,
            'current_task': asdict(self._current_task) if self._current_task else None,
        }

"""Task management service."""
import asyncio
import logging
import uuid
from datetime import datetime
from typing import Any, Dict, List, Optional

from database import db
from database.models import Task, TaskStatus, Priority, Agent

logger = logging.getLogger(__name__)

class TaskService:
    """Central service for creating, assigning, and tracking tasks."""

    def __init__(self):
        self._callbacks: List[Any] = []

    def on_event(self, callback):
        self._callbacks.append(callback)

    def _notify(self, event_type: str, data: Dict):
        for cb in self._callbacks:
            try:
                cb(event_type, data)
            except Exception:
                pass

    def create_task(
        self,
        title: str,
        description: str = '',
        priority: str = 'medium',
        agent_id: Optional[int] = None,
        project_id: Optional[int] = None,
        dependencies: List[str] = None,
    ) -> Task:
        """Create a new task in the database."""
        priority_map = {
            'critical': Priority.CRITICAL,
            'high': Priority.HIGH,
            'medium': Priority.MEDIUM,
            'low': Priority.LOW,
        }

        task = Task(
            title=title,
            description=description,
            priority=priority_map.get(priority.lower(), Priority.MEDIUM),
            status=TaskStatus.PENDING,
            agent_id=agent_id,
            project_id=project_id,
            dependencies=dependencies or [],
        )

        db.session.add(task)
        db.session.commit()

        self._notify('task_created', {
            'id': task.id,
            'title': task.title,
            'status': task.status.value,
        })

        logger.info(f"Created task {task.id}: {task.title}")
        return task

    def get_task(self, task_id: int) -> Optional[Task]:
        return Task.query.get(task_id)

    def list_tasks(
        self,
        status: Optional[str] = None,
        agent_id: Optional[int] = None,
        project_id: Optional[int] = None,
    ) -> List[Task]:
        """List tasks with optional filters."""
        query = Task.query

        if status:
            query = query.filter(Task.status == TaskStatus(status))
        if agent_id:
            query = query.filter(Task.agent_id == agent_id)
        if project_id:
            query = query.filter(Task.project_id == project_id)

        return query.order_by(Task.created_at.desc()).all()

    def update_task_status(self, task_id: int, status: str, result: Any = None, error: str = None) -> bool:
        """Update task status and optionally set result/error."""
        task = self.get_task(task_id)
        if not task:
            return False

        task.status = TaskStatus(status)

        if result is not None:
            task.result = str(result)[:10000]  # Limit result size
        if error:
            task.error = error[:5000]

        if status in ('completed', 'failed', 'cancelled'):
            task.finished_at = datetime.utcnow()

        db.session.commit()

        self._notify('task_updated', {
            'id': task.id,
            'status': status,
            'title': task.title,
        })

        return True

    def assign_task(self, task_id: int, agent_id: int) -> bool:
        """Assign a task to an agent."""
        task = self.get_task(task_id)
        if not task:
            return False

        task.agent_id = agent_id
        task.status = TaskStatus.QUEUED
        db.session.commit()

        self._notify('task_assigned', {
            'task_id': task_id,
            'agent_id': agent_id,
        })

        return True

    def delete_task(self, task_id: int) -> bool:
        task = self.get_task(task_id)
        if not task:
            return False

        db.session.delete(task)
        db.session.commit()
        return True

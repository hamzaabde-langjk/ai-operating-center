"""Notification Agent - Sends alerts and notifications."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class NotificationAgent(AgentBase):
    """The Notification agent handles all alerts, emails, and messages."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Notification Agent. Your role is to:"
            "1. Format and send notifications"
            "2. Manage notification preferences"
            "3. Route alerts to appropriate channels"
            "4. Batch and throttle notifications"
            "5. Track notification delivery"
            "Be timely but avoid notification fatigue."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Notification: {task.title}")

        notification = {
            'title': task.title,
            'message': task.description,
            'severity': 'info',
            'timestamp': task.created_at,
            'channels': ['dashboard', 'log'],
        }

        # Parse severity from task
        if 'critical' in task.description.lower() or 'error' in task.description.lower():
            notification['severity'] = 'critical'
        elif 'warning' in task.description.lower():
            notification['severity'] = 'warning'

        self._notify('notification', notification)

        return {
            'sent': True,
            'notification': notification,
        }

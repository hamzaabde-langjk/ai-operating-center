"""Scheduler Agent - Manages task scheduling and timing."""
import json
import logging
from datetime import datetime, timedelta
from typing import Any, Dict, List
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class SchedulerAgent(AgentBase):
    """The Scheduler agent manages cron jobs, timed tasks, and scheduling logic."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.scheduled_tasks: List[Dict] = []

    def _default_system_prompt(self) -> str:
        return (
            "You are the Scheduler Agent. Your role is to:
"
            "1. Schedule tasks at specific times
"
            "2. Manage recurring jobs
"
            "3. Handle timezone conversions
"
            "4. Resolve scheduling conflicts
"
            "5. Optimize task ordering
"
            "Be precise with timing and handle edge cases."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Scheduling: {task.title}")

        prompt = f"""
Create a schedule for:
Task: {task.title}
Description: {task.description}

Current time: {datetime.utcnow().isoformat()}

Provide:
1. Optimal start time
2. Estimated duration
3. Dependencies to resolve first
4. Resource requirements
5. Recommended execution order
"""

        response = await self.think(prompt)

        schedule = {
            'task': task.title,
            'schedule_analysis': response,
            'recommended_start': datetime.utcnow().isoformat(),
            'estimated_duration_minutes': 30,
        }

        self.scheduled_tasks.append(schedule)

        return schedule

"""Planner Agent - Breaks goals into actionable tasks."""
import json
import logging
from typing import Any, Dict, List
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class PlannerAgent(AgentBase):
    """The Planner agent breaks down high-level goals into detailed,
    actionable tasks with dependencies and priorities."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Planner Agent. Your role is to:"
            "1. Break down complex goals into small, actionable tasks"
            "2. Identify dependencies between tasks"
            "3. Assign priorities (critical, high, medium, low)"
            "4. Estimate effort for each task"
            "5. Create a logical execution sequence"
            "Be thorough, logical, and always consider edge cases."
        )

    async def execute(self, task: AgentTask) -> Any:
        """Execute planning task: break goal into subtasks."""
        self.log(f"Planning tasks for: {task.title}")

        prompt = f"""
Create a detailed task breakdown for:
Goal: {task.title}
Description: {task.description}

Provide a JSON array of tasks, each with:
- title: task name
- description: detailed description
- priority: 1-4 (1=critical, 4=low)
- assigned_role: which agent type should handle this
- dependencies: list of task titles this depends on
- estimated_effort: time estimate in minutes

Example:
{{
  "tasks": [
    {{
      "title": "Research requirements",
      "description": "Gather all requirements",
      "priority": 1,
      "assigned_role": "research",
      "dependencies": [],
      "estimated_effort": 30
    }}
  ]
}}
"""

        response = await self.think(prompt)

        try:
            plan = json.loads(response)
            tasks = plan.get('tasks', [])
        except json.JSONDecodeError:
            tasks = [{
                'title': task.title,
                'description': task.description,
                'priority': 2,
                'assigned_role': 'coding',
                'dependencies': [],
                'estimated_effort': 60,
            }]

        self.log(f"Created {len(tasks)} planned tasks")
        return {'tasks': tasks, 'total_estimated_effort': sum(t.get('estimated_effort', 0) for t in tasks)}

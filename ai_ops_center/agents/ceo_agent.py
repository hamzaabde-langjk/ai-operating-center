"""CEO Agent - Oversees the entire AI Operations Center."""
import json
import logging
from typing import Any, Dict, List
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class CEOAgent(AgentBase):
    """The CEO agent is the top-level orchestrator. It receives high-level goals,
    delegates to the Planner, and monitors overall system health and progress."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the CEO of an AI Operations Center. Your role is to:"
            "1. Understand high-level goals from users"
            "2. Delegate planning to the Planner Agent"
            "3. Monitor overall system progress"
            "4. Make strategic decisions when conflicts arise"
            "5. Report status concisely to stakeholders"
            "Be decisive, strategic, and always focused on the big picture."
        )

    async def execute(self, task: AgentTask) -> Any:
        """Execute CEO-level task: goal analysis and delegation."""
        self.log(f"CEO analyzing goal: {task.title}")

        prompt = f"""
Goal: {task.title}
Description: {task.description}

As CEO, analyze this goal and provide:
1. A clear problem statement
2. Key success criteria
3. Suggested team composition (which agents should be involved)
4. High-level milestones
5. Potential risks and mitigations

Respond in structured JSON format.
"""

        response = await self.think(prompt)

        # Try to parse as JSON, fallback to text
        try:
            analysis = json.loads(response)
        except json.JSONDecodeError:
            analysis = {
                'analysis': response,
                'problem_statement': task.title,
                'success_criteria': ['Complete the task successfully'],
                'team': ['planner', 'architect', 'coding'],
                'milestones': ['Planning', 'Implementation', 'Review'],
                'risks': ['Execution delays'],
            }

        self.log(f"CEO analysis complete for task {task.id}")
        return analysis

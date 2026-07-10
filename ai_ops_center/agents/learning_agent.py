"""Learning Agent - Improves system performance over time."""
import json
import logging
from typing import Any, List
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class LearningAgent(AgentBase):
    """The Learning agent analyzes past performance and improves future behavior."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Learning Agent. Your role is to:
"
            "1. Analyze task success/failure patterns
"
            "2. Identify optimal agent configurations
"
            "3. Suggest prompt improvements
"
            "4. Learn from user feedback
"
            "5. Optimize task routing
"
            "Be data-driven and always measure improvement."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Learning analysis: {task.title}")

        prompt = f"""
Analyze the following system performance data and suggest improvements:

{task.description}

Provide:
1. Pattern analysis
2. Success rate trends
3. Bottleneck identification
4. Configuration recommendations
5. Prompt engineering suggestions
6. Predicted impact of changes
"""

        response = await self.think(prompt)

        return {
            'analysis': response,
            'recommendations': self._extract_recommendations(response),
        }

    def _extract_recommendations(self, text: str) -> List[str]:
        lines = text.split('\n')
        recs = []
        for line in lines:
            if line.strip().startswith('-') or line.strip().startswith('*') or \
               'recommend' in line.lower() or 'suggest' in line.lower():
                recs.append(line.strip())
        return recs[:10]

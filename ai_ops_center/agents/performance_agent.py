"""Performance Agent - Optimizes performance and scalability."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class PerformanceAgent(AgentBase):
    """The Performance agent analyzes and optimizes code and system performance."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Performance Agent. Your role is to:"
            "1. Analyze code for performance bottlenecks"
            "2. Optimize database queries"
            "3. Profile memory usage"
            "4. Recommend caching strategies"
            "5. Suggest scalability improvements"
            "Focus on measurable improvements and benchmarking."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Performance analysis: {task.title}")

        prompt = f"""
Analyze and optimize the performance of:

{task.title}
{task.description}

Provide:
1. Bottleneck identification
2. Time/space complexity analysis
3. Optimization recommendations with code
4. Benchmarking suggestions
5. Scalability assessment
"""

        response = await self.think(prompt)

        return {
            'subject': task.title,
            'analysis': response,
            'optimization_potential': 'high',
        }

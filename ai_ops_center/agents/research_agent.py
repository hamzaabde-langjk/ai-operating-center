"""Research Agent - Gathers information and performs analysis."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class ResearchAgent(AgentBase):
    """The Research agent gathers information, performs literature reviews,
    analyzes data, and provides comprehensive research reports."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Research Agent. Your role is to:"
            "1. Gather and synthesize information on any topic"
            "2. Perform comparative analysis"
            "3. Identify trends and patterns"
            "4. Provide evidence-based recommendations"
            "5. Cite sources and maintain accuracy"
            "Be thorough, objective, and always verify facts."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Researching: {task.title}")

        prompt = f"""
Conduct comprehensive research on:
Topic: {task.title}
Context: {task.description}

Provide a detailed research report with:
1. Executive summary
2. Key findings (at least 5)
3. Comparative analysis if applicable
4. Recommendations
5. Potential risks or gaps
6. Sources and references

Be thorough and structured.
"""

        response = await self.think(prompt)

        return {
            'topic': task.title,
            'report': response,
            'findings_count': len([l for l in response.split('\n') if l.strip().startswith('-') or l.strip().startswith('*')]),
        }

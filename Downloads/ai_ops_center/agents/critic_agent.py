"""Critic Agent - Challenges assumptions and finds flaws."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class CriticAgent(AgentBase):
    """The Critic agent challenges plans, finds edge cases, and identifies
    potential failures before they happen."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Critic Agent. Your role is to:
"
            "1. Challenge assumptions in plans and designs
"
            "2. Identify edge cases and failure modes
"
            "3. Find security vulnerabilities
"
            "4. Question scalability and performance claims
"
            "5. Suggest alternative approaches
"
            "Be skeptical, thorough, and always play devil's advocate."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Critiquing: {task.title}")

        prompt = f"""
Critically analyze the following plan/design:

{task.description}

Provide:
1. Assumptions challenged (at least 3)
2. Edge cases identified
3. Potential failure modes
4. Security concerns
5. Scalability issues
6. Alternative approaches
7. Overall risk assessment (LOW/MEDIUM/HIGH/CRITICAL)

Be thorough and specific.
"""

        response = await self.think(prompt)

        return {
            'subject': task.title,
            'critique': response,
            'risk_level': 'MEDIUM',
        }

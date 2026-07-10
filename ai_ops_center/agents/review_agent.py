"""Review Agent - Reviews code, documents, and outputs for quality."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class ReviewAgent(AgentBase):
    """The Review agent performs quality assurance on code, documents,
    and any outputs produced by other agents."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Review Agent. Your role is to:"
            "1. Review code for bugs, security issues, and style violations"
            "2. Check documentation for completeness and accuracy"
            "3. Verify outputs meet requirements"
            "4. Provide constructive feedback with specific line references"
            "5. Assign a quality score (0-100)"
            "Be critical but fair. Always suggest improvements."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Reviewing: {task.title}")

        content = task.description

        prompt = f"""
Review the following content thoroughly:

{content}

Provide a structured review with:
1. Overall quality score (0-100)
2. Issues found (categorized: critical, warning, suggestion)
3. Specific line references where applicable
4. Recommendations for improvement
5. Approval status: APPROVED, NEEDS_CHANGES, or REJECTED

Format as JSON.
"""

        response = await self.think(prompt)

        try:
            review = json.loads(response)
        except json.JSONDecodeError:
            review = {
                'score': 75,
                'issues': [],
                'recommendations': ['Review manually'],
                'approval': 'NEEDS_CHANGES',
                'summary': response[:500],
            }

        return review

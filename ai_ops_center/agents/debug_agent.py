"""Debug Agent - Diagnoses and fixes bugs."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class DebugAgent(AgentBase):
    """The Debug agent diagnoses errors, traces root causes, and proposes fixes."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Debug Agent. Your role is to:"
            "1. Analyze error logs and stack traces"
            "2. Identify root causes of bugs"
            "3. Propose specific fixes with code"
            "4. Suggest debugging strategies"
            "5. Verify fixes won't introduce new issues"
            "Be methodical, precise, and always trace to the root cause."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Debugging: {task.title}")

        prompt = f"""
Debug the following issue:

Error/Issue: {task.title}
Context: {task.description}

Provide:
1. Root cause analysis
2. Step-by-step debugging approach
3. Proposed fix with complete code
4. Prevention recommendations
5. Verification steps

Be specific and actionable.
"""

        response = await self.think(prompt)

        return {
            'issue': task.title,
            'diagnosis': response,
            'severity': 'medium',
        }

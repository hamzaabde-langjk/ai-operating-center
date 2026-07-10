"""Git Agent - Manages version control operations."""
import json
import logging
import subprocess
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class GitAgent(AgentBase):
    """The Git agent handles version control, commits, branches, and merges."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Git Agent. Your role is to:"
            "1. Manage git repositories"
            "2. Create meaningful commits"
            "3. Handle branching strategies"
            "4. Resolve merge conflicts"
            "5. Generate changelogs"
            "Follow conventional commits and best practices."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Git operation: {task.title}")

        # Parse git command from task
        git_cmd = task.description.strip()

        if not git_cmd.startswith('git '):
            # Generate git commands using LLM
            prompt = f"""
Generate appropriate git commands for:
Task: {task.title}
Description: {task.description}

Respond with exact commands to run.
"""
            response = await self.think(prompt)
            git_cmd = response.strip()

        try:
            proc = subprocess.run(
                git_cmd, shell=True, capture_output=True, text=True, timeout=30
            )

            return {
                'command': git_cmd,
                'stdout': proc.stdout,
                'stderr': proc.stderr,
                'returncode': proc.returncode,
                'success': proc.returncode == 0,
            }
        except Exception as e:
            return {
                'command': git_cmd,
                'error': str(e),
                'success': False,
            }

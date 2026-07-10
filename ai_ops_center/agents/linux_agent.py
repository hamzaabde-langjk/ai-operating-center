"""Linux Agent - Executes Linux commands and manages systems."""
import json
import logging
import subprocess
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class LinuxAgent(AgentBase):
    """The Linux agent executes system commands, manages processes,
    and handles system administration tasks."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Linux Agent. Your role is to:"
            "1. Execute Linux commands safely"
            "2. Manage processes and services"
            "3. Configure system settings"
            "4. Monitor system health"
            "5. Handle file operations"
            "Always validate commands for safety before execution."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Linux task: {task.title}")

        prompt = f"""
Given the following Linux/system task, provide the exact commands to execute:

Task: {task.title}
Description: {task.description}

Respond with a JSON array of commands:
{{
  "commands": [
    {{"cmd": "command here", "description": "what this does", "safe": true}}
  ],
  "explanation": "overall explanation"
}}
"""

        response = await self.think(prompt)

        try:
            result = json.loads(response)
            commands = result.get('commands', [])
        except json.JSONDecodeError:
            commands = []
            result = {'explanation': response}

        executed = []
        for cmd_info in commands:
            cmd = cmd_info.get('cmd', '')
            if not cmd:
                continue
            try:
                proc = subprocess.run(
                    cmd, shell=True, capture_output=True, text=True, timeout=30
                )
                executed.append({
                    'command': cmd,
                    'stdout': proc.stdout,
                    'stderr': proc.stderr,
                    'returncode': proc.returncode,
                })
            except Exception as e:
                executed.append({
                    'command': cmd,
                    'error': str(e),
                })

        return {
            'task': task.title,
            'commands': commands,
            'executed': executed,
            'explanation': result.get('explanation', ''),
        }

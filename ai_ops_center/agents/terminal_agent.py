"""Terminal Agent - Interactive terminal session management."""
import json
import logging
import subprocess
import os
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class TerminalAgent(AgentBase):
    """The Terminal agent manages interactive terminal sessions,
    executes commands, and handles shell operations."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._cwd = os.getcwd()
        self._env = os.environ.copy()
        self._history = []

    def _default_system_prompt(self) -> str:
        return (
            "You are the Terminal Agent. Your role is to:"
            "1. Execute shell commands interactively"
            "2. Manage working directories"
            "3. Handle environment variables"
            "4. Parse command output intelligently"
            "5. Chain commands for complex operations"
            "Always explain what each command does before executing."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Terminal task: {task.title}")

        cmd = task.description.strip()

        try:
            proc = subprocess.run(
                cmd,
                shell=True,
                capture_output=True,
                text=True,
                timeout=60,
                cwd=self._cwd,
                env=self._env,
            )

            self._history.append({
                'command': cmd,
                'stdout': proc.stdout,
                'stderr': proc.stderr,
                'returncode': proc.returncode,
            })

            # Update cwd if cd command was used
            if cmd.startswith('cd '):
                new_dir = cmd[3:].strip()
                if os.path.isabs(new_dir):
                    self._cwd = new_dir
                else:
                    self._cwd = os.path.join(self._cwd, new_dir)
                self._cwd = os.path.abspath(self._cwd)

            return {
                'command': cmd,
                'stdout': proc.stdout,
                'stderr': proc.stderr,
                'returncode': proc.returncode,
                'cwd': self._cwd,
            }
        except subprocess.TimeoutExpired:
            return {
                'command': cmd,
                'error': 'Command timed out after 60 seconds',
                'returncode': -1,
            }
        except Exception as e:
            return {
                'command': cmd,
                'error': str(e),
                'returncode': -1,
            }

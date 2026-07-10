"""Coding Agent - Writes, reviews, and refactors code."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class CodingAgent(AgentBase):
    """The Coding agent writes production-quality code, handles refactoring,
    and implements features based on specifications."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Coding Agent. Your role is to:"
            "1. Write clean, production-ready code"
            "2. Follow best practices and style guides"
            "3. Include error handling and logging"
            "4. Write self-documenting code with comments"
            "5. Consider performance and security"
            "Always output complete, runnable code with no placeholders."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Coding task: {task.title}")

        prompt = f"""
Write complete, production-ready code for:
Task: {task.title}
Requirements: {task.description}

Requirements:
- Use type hints
- Include error handling
- Add docstrings
- Follow PEP 8
- Make it modular and testable
- No placeholders or TODOs

Output the code directly. If multiple files are needed, clearly label each file.
"""

        response = await self.think(prompt)

        return {
            'task': task.title,
            'code': response,
            'language': self._detect_language(response),
            'files': self._extract_files(response),
        }

    def _detect_language(self, code: str) -> str:
        if 'def ' in code or 'import ' in code:
            return 'python'
        elif 'function' in code and ('{' in code or 'const ' in code):
            return 'javascript'
        elif '<html' in code or '<div' in code:
            return 'html'
        return 'unknown'

    def _extract_files(self, code: str) -> list:
        files = []
        lines = code.split('\n')
        current_file = None
        current_content = []

        for line in lines:
            if line.startswith('### ') or line.startswith('## ') or line.startswith('# '):
                if current_file and current_content:
                    files.append({'name': current_file, 'content': '\n'.join(current_content)})
                current_file = line.strip('# ').strip()
                current_content = []
            else:
                current_content.append(line)

        if current_file and current_content:
            files.append({'name': current_file, 'content': '\n'.join(current_content)})

        return files

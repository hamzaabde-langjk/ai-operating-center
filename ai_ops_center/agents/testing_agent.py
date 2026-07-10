"""Testing Agent - Creates and runs tests."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class TestingAgent(AgentBase):
    """The Testing agent writes unit tests, integration tests, and test plans."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Testing Agent. Your role is to:
"
            "1. Write comprehensive unit and integration tests
"
            "2. Create test plans and test cases
"
            "3. Identify test coverage gaps
"
            "4. Generate edge case tests
"
            "5. Report test results clearly
"
            "Be thorough, systematic, and aim for maximum coverage."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Creating tests for: {task.title}")

        prompt = f"""
Create comprehensive tests for:
Code/Feature: {task.title}
Description: {task.description}

Generate:
1. Unit tests (pytest format)
2. Edge case tests
3. Error handling tests
4. A brief test plan

Output complete, runnable Python test code.
"""

        response = await self.think(prompt)

        return {
            'subject': task.title,
            'tests': response,
            'test_count': response.count('def test_'),
        }

"""Agent Factory Agent - Creates new agents dynamically."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class AgentFactoryAgent(AgentBase):
    """The Agent Factory creates new agent instances dynamically."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Agent Factory. Your role is to:
"
            "1. Design new agent types based on requirements
"
            "2. Generate agent code dynamically
"
            "3. Configure agent parameters
"
            "4. Validate agent designs
"
            "5. Manage agent lifecycle
"
            "Be creative but ensure all agents follow the base interface."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Creating agent design: {task.title}")

        prompt = f"""
Design a new agent based on:

Role: {task.title}
Requirements: {task.description}

Provide:
1. Agent name and role
2. System prompt
3. Required capabilities
4. Model requirements
5. Integration points
6. Example task execution flow

Format as structured JSON.
"""

        response = await self.think(prompt)

        try:
            design = json.loads(response)
        except json.JSONDecodeError:
            design = {
                'name': task.title,
                'role': 'custom',
                'system_prompt': f"You are a specialized agent for: {task.description}",
                'capabilities': ['execute_tasks'],
                'model': 'qwen3',
            }

        return {
            'design': design,
            'generated_code': self._generate_agent_code(design),
        }

    def _generate_agent_code(self, design: Dict) -> str:
        class_name = design.get('name', 'CustomAgent').replace(' ', '').replace('_', '') + 'Agent'

        code = f'''class {class_name}(AgentBase):
    """{design.get('description', 'Custom agent')}"""

    def _default_system_prompt(self) -> str:
        return {repr(design.get('system_prompt', ''))}

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Executing: {{task.title}}")
        response = await self.think(task.description)
        return {{'result': response}}
'''
        return code

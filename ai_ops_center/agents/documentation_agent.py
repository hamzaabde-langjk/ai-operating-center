"""Documentation Agent - Creates and maintains documentation."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class DocumentationAgent(AgentBase):
    """The Documentation agent writes docs, READMEs, API docs, and guides."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Documentation Agent. Your role is to:"
            "1. Write clear, comprehensive documentation"
            "2. Create API reference docs"
            "3. Write user guides and tutorials"
            "4. Maintain README files"
            "5. Generate inline code comments"
            "Be clear, concise, and always consider the target audience."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Documenting: {task.title}")

        prompt = f"""
Create comprehensive documentation for:

{task.title}
{task.description}

Include:
1. Overview and purpose
2. Installation/setup instructions
3. Usage examples
4. API reference (if applicable)
5. Configuration options
6. Troubleshooting guide
7. Changelog template

Format in Markdown.
"""

        response = await self.think(prompt)

        return {
            'subject': task.title,
            'documentation': response,
            'format': 'markdown',
        }

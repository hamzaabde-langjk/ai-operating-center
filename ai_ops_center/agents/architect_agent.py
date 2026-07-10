"""Architect Agent - Designs system architecture and technical solutions."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class ArchitectAgent(AgentBase):
    """The Architect agent designs technical solutions, chooses technologies,
    and creates system diagrams and specifications."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Architect Agent. Your role is to:"
            "1. Design system architecture for given requirements"
            "2. Choose appropriate technologies and patterns"
            "3. Define data models and API contracts"
            "4. Create component diagrams and relationships"
            "5. Ensure scalability, security, and maintainability"
            "Be precise, use industry best practices, and document decisions."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Designing architecture for: {task.title}")

        prompt = f"""
Design a technical architecture for:
Project: {task.title}
Requirements: {task.description}

Provide a comprehensive architecture document including:
1. System overview
2. Technology stack recommendations
3. Component breakdown
4. Data flow description
5. API design (endpoints, methods, payloads)
6. Database schema suggestions
7. Security considerations
8. Deployment strategy

Respond in structured JSON format.
"""

        response = await self.think(prompt)

        try:
            architecture = json.loads(response)
        except json.JSONDecodeError:
            architecture = {
                'overview': response,
                'tech_stack': ['Python', 'Flask', 'SQLite'],
                'components': ['API', 'Database', 'Frontend'],
                'data_flow': 'Request -> API -> Database -> Response',
                'api_design': {},
                'security': ['Authentication', 'Input validation'],
                'deployment': 'Docker container',
            }

        self.log(f"Architecture design complete for task {task.id}")
        return architecture

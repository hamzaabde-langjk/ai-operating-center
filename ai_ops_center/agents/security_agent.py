"""Security Agent - Performs security audits and hardening."""
import json
import logging
from typing import Any
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class SecurityAgent(AgentBase):
    """The Security agent audits code, configurations, and systems for vulnerabilities."""

    def _default_system_prompt(self) -> str:
        return (
            "You are the Security Agent. Your role is to:"
            "1. Audit code for security vulnerabilities"
            "2. Review configurations for security misconfigurations"
            "3. Check for OWASP Top 10 issues"
            "4. Verify authentication and authorization"
            "5. Recommend security hardening measures"
            "Be paranoid, thorough, and always assume breach."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Security audit: {task.title}")

        prompt = f"""
Perform a comprehensive security audit of:

{task.title}
{task.description}

Check for:
1. Injection vulnerabilities (SQL, command, etc.)
2. Authentication/authorization flaws
3. Sensitive data exposure
4. XSS and CSRF vulnerabilities
5. Security misconfigurations
6. Insecure dependencies
7. Input validation issues
8. Logging and monitoring gaps

Provide severity ratings (CRITICAL/HIGH/MEDIUM/LOW) for each finding.
"""

        response = await self.think(prompt)

        return {
            'audit_subject': task.title,
            'findings': response,
            'risk_score': 'pending_manual_review',
        }

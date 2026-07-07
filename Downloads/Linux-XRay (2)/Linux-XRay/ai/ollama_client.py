#!/usr/bin/env python3
"""
Ollama Client for Natural Language Explanations
"""

import requests
import json

class OllamaClient:
    def __init__(self, base_url='http://localhost:11434', model='llama2'):
        self.base_url = base_url
        self.model = model

    def explain_event(self, event):
        """Generate natural language explanation of an event."""
        prompt = self._build_prompt(event)

        try:
            response = requests.post(
                f'{self.base_url}/api/generate',
                json={
                    'model': self.model,
                    'prompt': prompt,
                    'stream': False
                },
                timeout=30
            )

            if response.status_code == 200:
                return response.json().get('response', 'No explanation available')
            else:
                return f'Error: HTTP {response.status_code}'
        except requests.exceptions.ConnectionError:
            return 'Ollama server not available. Start with: ollama run llama2'
        except Exception as e:
            return f'Error: {str(e)}'

    def _build_prompt(self, event):
        """Build a prompt for the LLM."""
        return f"""Analyze this system event and provide a brief security assessment:

Event Type: {event.get('type', 'Unknown')}
Process: {event.get('process_name', 'Unknown')} (PID: {event.get('pid', 0)})
User: {event.get('user', 'Unknown')}
Path: {event.get('path', 'N/A')}
Timestamp: {event.get('timestamp', 'Unknown')}

Provide a one-sentence explanation of what this event means and whether it could be suspicious."""

    def summarize_alerts(self, alerts):
        """Generate summary of multiple alerts."""
        alert_text = '\n'.join([
            f"- {a['type']}: {a.get('description', 'No description')}"
            for a in alerts[:10]
        ])

        prompt = f"""Summarize the following security alerts and identify the most critical threats:

{alert_text}

Provide a brief summary and recommended actions."""

        return self._query(prompt)

    def _query(self, prompt):
        """Send query to Ollama."""
        try:
            response = requests.post(
                f'{self.base_url}/api/generate',
                json={'model': self.model, 'prompt': prompt, 'stream': False},
                timeout=60
            )
            return response.json().get('response', 'No response')
        except Exception as e:
            return f'Error: {str(e)}'

"""Plugin Manager - Discovers and manages plugins."""
import json
import logging
import os
import importlib.util
from typing import Any, Dict, List, Type
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class PluginManager(AgentBase):
    """The Plugin Manager discovers, loads, and manages agent plugins."""

    def __init__(self, *args, plugins_dir: str = 'plugins', **kwargs):
        super().__init__(*args, **kwargs)
        self.plugins_dir = plugins_dir
        self.loaded_plugins: Dict[str, Any] = {}

    def _default_system_prompt(self) -> str:
        return (
            "You are the Plugin Manager. Your role is to:
"
            "1. Discover plugins in the plugins directory
"
            "2. Validate plugin compatibility
"
            "3. Load and unload plugins dynamically
"
            "4. Monitor plugin health
"
            "5. Handle plugin conflicts
"
            "Be careful with security when loading external code."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Plugin operation: {task.title}")

        if 'discover' in task.title.lower() or 'scan' in task.title.lower():
            return self._discover_plugins()
        elif 'load' in task.title.lower():
            return await self._load_plugin(task.description)
        else:
            return self._discover_plugins()

    def _discover_plugins(self) -> Dict:
        discovered = []

        if not os.path.exists(self.plugins_dir):
            return {'plugins': [], 'directory': self.plugins_dir}

        for item in os.listdir(self.plugins_dir):
            plugin_path = os.path.join(self.plugins_dir, item)
            if os.path.isdir(plugin_path):
                init_file = os.path.join(plugin_path, '__init__.py')
                plugin_file = os.path.join(plugin_path, 'plugin.py')

                if os.path.exists(init_file) or os.path.exists(plugin_file):
                    discovered.append({
                        'name': item,
                        'path': plugin_path,
                        'has_init': os.path.exists(init_file),
                        'has_plugin': os.path.exists(plugin_file),
                    })

        return {
            'plugins': discovered,
            'count': len(discovered),
            'directory': self.plugins_dir,
        }

    async def _load_plugin(self, plugin_name: str) -> Dict:
        return {
            'plugin': plugin_name,
            'loaded': False,
            'reason': 'Dynamic loading requires additional security review',
        }

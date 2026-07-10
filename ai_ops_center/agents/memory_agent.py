"""Memory Agent - Manages short-term and long-term memory."""
import json
import logging
from typing import Any, List, Dict
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class MemoryAgent(AgentBase):
    """The Memory agent manages all memory types: short, long, shared, and vector."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.short_term: List[Dict] = []
        self.long_term: List[Dict] = []
        self.shared: Dict[str, Any] = {}
        self.vector_store: List[Dict] = []

    def _default_system_prompt(self) -> str:
        return (
            "You are the Memory Agent. Your role is to:"
            "1. Store and retrieve short-term memories"
            "2. Consolidate important memories to long-term"
            "3. Manage shared knowledge between agents"
            "4. Perform semantic search on memories"
            "5. Forget irrelevant or outdated information"
            "Be organized and prioritize relevance."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Memory operation: {task.title}")

        operation = task.title.lower()

        if 'store' in operation or 'save' in operation:
            return await self._store_memory(task)
        elif 'retrieve' in operation or 'search' in operation or 'find' in operation:
            return await self._retrieve_memory(task)
        elif 'consolidate' in operation:
            return await self._consolidate_memory()
        elif 'clear' in operation:
            return self._clear_memory()
        else:
            return await self._store_memory(task)

    async def _store_memory(self, task: AgentTask) -> Dict:
        entry = {
            'id': task.id,
            'content': task.description,
            'timestamp': task.created_at,
            'type': 'short_term',
        }
        self.short_term.append(entry)

        # Keep short term limited
        if len(self.short_term) > 100:
            old = self.short_term.pop(0)
            # Promote to long term if relevant
            self.long_term.append({**old, 'type': 'long_term'})

        return {'stored': True, 'memory_id': task.id, 'short_term_size': len(self.short_term)}

    async def _retrieve_memory(self, task: AgentTask) -> Dict:
        query = task.description.lower()
        results = []

        for mem in self.short_term + self.long_term:
            score = self._relevance_score(query, mem.get('content', ''))
            if score > 0.3:
                results.append({**mem, 'score': score})

        results.sort(key=lambda x: x['score'], reverse=True)
        return {'results': results[:10], 'query': query}

    def _relevance_score(self, query: str, content: str) -> float:
        query_words = set(query.lower().split())
        content_words = set(content.lower().split())
        if not query_words:
            return 0.0
        intersection = query_words & content_words
        return len(intersection) / len(query_words)

    async def _consolidate_memory(self) -> Dict:
        # Move important short-term to long-term
        consolidated = 0
        for mem in self.short_term[:]:
            if len(mem.get('content', '')) > 50:  # Heuristic for importance
                self.long_term.append({**mem, 'type': 'long_term'})
                self.short_term.remove(mem)
                consolidated += 1

        return {'consolidated': consolidated, 'long_term_size': len(self.long_term)}

    def _clear_memory(self) -> Dict:
        self.short_term.clear()
        return {'cleared': True, 'short_term_size': 0}

"""Async Ollama client with multi-model support."""
import asyncio
import json
import logging
from typing import Optional, List, Dict, Any, AsyncGenerator
import ollama
from ollama import AsyncClient, ResponseError

logger = logging.getLogger(__name__)

class OllamaClient:
    """Production-ready async Ollama client supporting multiple models."""

    def __init__(self, host: str = 'http://localhost:11434', timeout: int = 120):
        self.host = host
        self.timeout = timeout
        self.client = AsyncClient(host=host)
        self._available_models: List[str] = []
        self._lock = asyncio.Lock()

    async def list_models(self) -> List[Dict[str, Any]]:
        """List available Ollama models."""
        try:
            response = await self.client.list()
            models = response.get('models', [])
            self._available_models = [m.get('model', m.get('name', '')) for m in models]
            return models
        except Exception as e:
            logger.error(f"Failed to list models: {e}")
            return []

    async def chat(
        self,
        model: str,
        messages: List[Dict[str, str]],
        temperature: float = 0.7,
        max_tokens: int = 4096,
        stream: bool = False,
        system: Optional[str] = None,
        tools: Optional[List[Dict]] = None,
    ) -> Dict[str, Any]:
        """Send a chat completion request."""
        try:
            options = {
                'temperature': temperature,
                'num_predict': max_tokens,
            }

            kwargs = {
                'model': model,
                'messages': messages,
                'options': options,
                'stream': stream,
            }
            if system:
                kwargs['system'] = system
            if tools:
                kwargs['tools'] = tools

            if stream:
                return await self.client.chat(**kwargs)

            response = await self.client.chat(**kwargs)
            return {
                'content': response.message.content,
                'model': model,
                'done': response.done,
                'usage': {
                    'prompt_tokens': response.prompt_eval_count,
                    'completion_tokens': response.eval_count,
                    'total_tokens': (response.prompt_eval_count or 0) + (response.eval_count or 0),
                }
            }
        except ResponseError as e:
            logger.error(f"Ollama ResponseError [{e.status_code}]: {e.error}")
            raise
        except Exception as e:
            logger.error(f"Ollama chat error: {e}")
            raise

    async def generate(
        self,
        model: str,
        prompt: str,
        temperature: float = 0.7,
        max_tokens: int = 4096,
        system: Optional[str] = None,
    ) -> Dict[str, Any]:
        """Generate text from a prompt."""
        try:
            options = {
                'temperature': temperature,
                'num_predict': max_tokens,
            }
            response = await self.client.generate(
                model=model,
                prompt=prompt,
                system=system or '',
                options=options,
            )
            return {
                'content': response.response,
                'model': model,
                'done': response.done,
                'usage': {
                    'prompt_tokens': response.prompt_eval_count,
                    'completion_tokens': response.eval_count,
                    'total_tokens': (response.prompt_eval_count or 0) + (response.eval_count or 0),
                }
            }
        except Exception as e:
            logger.error(f"Ollama generate error: {e}")
            raise

    async def embed(self, model: str, input_text: str) -> List[float]:
        """Generate embeddings for text."""
        try:
            response = await self.client.embed(model=model, input=input_text)
            return response.embeddings[0] if response.embeddings else []
        except Exception as e:
            logger.error(f"Ollama embed error: {e}")
            return []

    async def is_model_available(self, model: str) -> bool:
        """Check if a model is available locally."""
        if not self._available_models:
            await self.list_models()
        return model in self._available_models

    async def pull_model(self, model: str) -> bool:
        """Pull a model from Ollama registry."""
        try:
            async for progress in self.client.pull(model, stream=True):
                logger.info(f"Pulling {model}: {progress}")
            return True
        except Exception as e:
            logger.error(f"Failed to pull model {model}: {e}")
            return False

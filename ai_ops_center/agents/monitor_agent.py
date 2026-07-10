"""Monitor Agent - Monitors system health and resources."""
import json
import logging
import psutil
from typing import Any, Dict
from datetime import datetime
from core.agent_base import AgentBase, AgentTask

logger = logging.getLogger(__name__)

class MonitorAgent(AgentBase):
    """The Monitor agent tracks CPU, RAM, disk, network, and GPU usage."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._metrics_history: list = []

    def _default_system_prompt(self) -> str:
        return (
            "You are the Monitor Agent. Your role is to:
"
            "1. Track system resource usage
"
            "2. Alert on threshold breaches
"
            "3. Generate performance reports
"
            "4. Predict resource needs
"
            "5. Recommend scaling actions
"
            "Be vigilant and proactive about issues."
        )

    async def execute(self, task: AgentTask) -> Any:
        self.log(f"Monitoring: {task.title}")

        metrics = self._collect_metrics()
        self._metrics_history.append({**metrics, 'timestamp': datetime.utcnow().isoformat()})

        # Keep history limited
        if len(self._metrics_history) > 1000:
            self._metrics_history = self._metrics_history[-500:]

        # Check thresholds
        alerts = self._check_thresholds(metrics)

        return {
            'metrics': metrics,
            'alerts': alerts,
            'history_size': len(self._metrics_history),
        }

    def _collect_metrics(self) -> Dict:
        metrics = {
            'cpu': {
                'percent': psutil.cpu_percent(interval=1),
                'count': psutil.cpu_count(),
                'freq': psutil.cpu_freq()._asdict() if psutil.cpu_freq() else None,
            },
            'memory': {
                'total': psutil.virtual_memory().total,
                'available': psutil.virtual_memory().available,
                'percent': psutil.virtual_memory().percent,
                'used': psutil.virtual_memory().used,
            },
            'disk': {
                'total': psutil.disk_usage('/').total,
                'used': psutil.disk_usage('/').used,
                'free': psutil.disk_usage('/').free,
                'percent': psutil.disk_usage('/').percent,
            },
            'network': {
                'bytes_sent': psutil.net_io_counters().bytes_sent,
                'bytes_recv': psutil.net_io_counters().bytes_recv,
            },
        }

        # GPU metrics if available
        try:
            import pynvml
            pynvml.nvmlInit()
            handle = pynvml.nvmlDeviceGetHandleByIndex(0)
            gpu_info = pynvml.nvmlDeviceGetUtilizationRates(handle)
            metrics['gpu'] = {
                'utilization': gpu_info.gpu,
                'memory': gpu_info.memory,
            }
        except Exception:
            metrics['gpu'] = None

        return metrics

    def _check_thresholds(self, metrics: Dict) -> list:
        alerts = []

        if metrics['cpu']['percent'] > 90:
            alerts.append({'severity': 'critical', 'metric': 'cpu', 'value': metrics['cpu']['percent']})
        elif metrics['cpu']['percent'] > 75:
            alerts.append({'severity': 'warning', 'metric': 'cpu', 'value': metrics['cpu']['percent']})

        if metrics['memory']['percent'] > 90:
            alerts.append({'severity': 'critical', 'metric': 'memory', 'value': metrics['memory']['percent']})
        elif metrics['memory']['percent'] > 80:
            alerts.append({'severity': 'warning', 'metric': 'memory', 'value': metrics['memory']['percent']})

        if metrics['disk']['percent'] > 90:
            alerts.append({'severity': 'critical', 'metric': 'disk', 'value': metrics['disk']['percent']})

        return alerts

"""System monitoring service."""
import asyncio
import logging
import psutil
import time
from datetime import datetime
from typing import Any, Dict, List

logger = logging.getLogger(__name__)

class MonitorService:
    """Continuous system monitoring with history."""

    def __init__(self, interval: int = 5):
        self.interval = interval
        self.history: List[Dict] = []
        self.max_history = 1000
        self._running = False

    def get_current_metrics(self) -> Dict[str, Any]:
        """Get current system metrics."""
        metrics = {
            'timestamp': datetime.utcnow().isoformat(),
            'cpu': {
                'percent': psutil.cpu_percent(interval=0.5),
                'count': psutil.cpu_count(),
                'freq': psutil.cpu_freq()._asdict() if psutil.cpu_freq() else None,
            },
            'memory': {
                'total': psutil.virtual_memory().total,
                'available': psutil.virtual_memory().available,
                'percent': psutil.virtual_memory().percent,
                'used': psutil.virtual_memory().used,
                'free': psutil.virtual_memory().free,
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
                'packets_sent': psutil.net_io_counters().packets_sent,
                'packets_recv': psutil.net_io_counters().packets_recv,
            },
            'processes': len(psutil.pids()),
            'boot_time': psutil.boot_time(),
        }

        # GPU
        try:
            import pynvml
            pynvml.nvmlInit()
            handle = pynvml.nvmlDeviceGetHandleByIndex(0)
            util = pynvml.nvmlDeviceGetUtilizationRates(handle)
            mem = pynvml.nvmlDeviceGetMemoryInfo(handle)
            metrics['gpu'] = {
                'utilization': util.gpu,
                'memory_used': mem.used,
                'memory_total': mem.total,
                'temperature': pynvml.nvmlDeviceGetTemperature(handle, pynvml.NVML_TEMPERATURE_GPU),
            }
        except Exception:
            metrics['gpu'] = None

        # Temperature sensors
        try:
            temps = psutil.sensors_temperatures()
            metrics['temperatures'] = {
                name: [{'label': t.label, 'current': t.current, 'high': t.high, 'critical': t.critical} 
                       for t in entries]
                for name, entries in temps.items()
            }
        except Exception:
            metrics['temperatures'] = {}

        return metrics

    async def start_monitoring(self):
        """Start continuous monitoring loop."""
        self._running = True
        while self._running:
            try:
                metrics = self.get_current_metrics()
                self.history.append(metrics)

                if len(self.history) > self.max_history:
                    self.history = self.history[-self.max_history:]

                await asyncio.sleep(self.interval)
            except Exception as e:
                logger.error(f"Monitor error: {e}")
                await asyncio.sleep(self.interval)

    def stop(self):
        self._running = False

    def get_history(self, limit: int = 100) -> List[Dict]:
        return self.history[-limit:]

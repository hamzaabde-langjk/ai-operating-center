#!/usr/bin/env python3
"""
Behavior Analysis Module
Clusters processes and identifies behavioral patterns.
"""

import numpy as np
from sklearn.cluster import DBSCAN
from sklearn.preprocessing import StandardScaler
import json

class BehaviorAnalyzer:
    def __init__(self, eps=0.5, min_samples=5):
        self.eps = eps
        self.min_samples = min_samples
        self.scaler = StandardScaler()
        self.clusters = {}

    def cluster_processes(self, process_data):
        """Cluster processes by behavior."""
        if len(process_data) < self.min_samples:
            return []

        features = np.array([
            [p['cpu_avg'], p['mem_avg'], p['io_read'], p['io_write'], p['net_connections']]
            for p in process_data
        ])

        scaled = self.scaler.fit_transform(features)
        clustering = DBSCAN(eps=self.eps, min_samples=self.min_samples).fit(scaled)

        results = []
        for i, label in enumerate(clustering.labels_):
            results.append({
                'pid': process_data[i]['pid'],
                'name': process_data[i]['name'],
                'cluster': int(label),
                'is_outlier': label == -1
            })

        return results

    def detect_behavior_change(self, process_history):
        """Detect significant changes in process behavior."""
        if len(process_history) < 2:
            return None

        recent = process_history[-10:]
        older = process_history[:-10] if len(process_history) > 10 else process_history[:1]

        recent_cpu = np.mean([p['cpu'] for p in recent])
        older_cpu = np.mean([p['cpu'] for p in older])

        if abs(recent_cpu - older_cpu) > 2 * np.std([p['cpu'] for p in process_history]):
            return {
                'type': 'cpu_spike',
                'change': float(recent_cpu - older_cpu),
                'severity': 'high' if abs(recent_cpu - older_cpu) > 50 else 'medium'
            }

        return None

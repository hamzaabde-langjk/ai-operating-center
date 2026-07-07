#!/usr/bin/env python3
"""
Anomaly Detection Module for Linux X-Ray Vision
Uses statistical and ML methods to detect unusual system behavior.
"""

import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
from collections import deque
import json
import sys

class AnomalyDetector:
    def __init__(self, window_size=1000, contamination=0.01):
        self.window_size = window_size
        self.contamination = contamination
        self.scaler = StandardScaler()
        self.model = IsolationForest(contamination=contamination, random_state=42)
        self.data_buffer = deque(maxlen=window_size)
        self.is_trained = False

    def add_sample(self, features):
        """Add a feature vector and return anomaly score."""
        self.data_buffer.append(features)

        if len(self.data_buffer) >= self.window_size and not self.is_trained:
            self._train()

        if self.is_trained:
            scaled = self.scaler.transform([features])
            score = self.model.decision_function(scaled)[0]
            is_anomaly = self.model.predict(scaled)[0] == -1
            return {
                'is_anomaly': bool(is_anomaly),
                'score': float(score),
                'confidence': float(1.0 - abs(score))
            }

        return {'is_anomaly': False, 'score': 0.0, 'confidence': 0.0}

    def _train(self):
        """Train the anomaly detection model."""
        data = np.array(list(self.data_buffer))
        self.scaler.fit(data)
        scaled = self.scaler.transform(data)
        self.model.fit(scaled)
        self.is_trained = True

    def analyze_event_stream(self, events):
        """Analyze a stream of events and return anomalies."""
        anomalies = []
        for event in events:
            features = self._extract_features(event)
            result = self.add_sample(features)
            if result['is_anomaly']:
                anomalies.append({
                    'event': event,
                    'score': result['score'],
                    'confidence': result['confidence']
                })
        return anomalies

    def _extract_features(self, event):
        """Extract numerical features from an event."""
        return [
            event.get('timestamp', 0) % 86400,
            hash(event.get('type', '')) % 1000,
            event.get('pid', 0),
            event.get('cpu_usage', 0),
            event.get('memory_usage', 0)
        ]

def main():
    detector = AnomalyDetector()

    for line in sys.stdin:
        try:
            event = json.loads(line)
            result = detector.add_sample(detector._extract_features(event))
            if result['is_anomaly']:
                print(json.dumps({
                    'alert': 'Anomaly detected',
                    'event': event,
                    'score': result['score']
                }))
        except json.JSONDecodeError:
            continue

if __name__ == '__main__':
    main()

"""REST API routes."""
from flask import Blueprint, request, jsonify
from flask_login import login_required, current_user

from database import db
from database.models import Project, Task, Agent, Event, LogEntry, MemoryEntry, Setting

api_bp = Blueprint('api', __name__)

@api_bp.route('/projects', methods=['GET', 'POST'])
@login_required
def projects():
    if request.method == 'POST':
        data = request.get_json()
        project = Project(
            name=data.get('name'),
            description=data.get('description', ''),
            goal=data.get('goal', ''),
        )
        db.session.add(project)
        db.session.commit()
        return jsonify({'id': project.id, 'name': project.name}), 201

    projects = Project.query.all()
    return jsonify([{
        'id': p.id,
        'name': p.name,
        'description': p.description,
        'status': p.status,
        'created_at': p.created_at.isoformat() if p.created_at else None,
    } for p in projects])

@api_bp.route('/tasks', methods=['GET', 'POST'])
@login_required
def tasks():
    if request.method == 'POST':
        data = request.get_json()
        task = Task(
            title=data.get('title'),
            description=data.get('description', ''),
            priority=data.get('priority', 'medium'),
            project_id=data.get('project_id'),
            agent_id=data.get('agent_id'),
        )
        db.session.add(task)
        db.session.commit()
        return jsonify({'id': task.id, 'title': task.title}), 201

    tasks = Task.query.all()
    return jsonify([{
        'id': t.id,
        'title': t.title,
        'status': t.status.value,
        'priority': t.priority.value,
        'created_at': t.created_at.isoformat() if t.created_at else None,
    } for t in tasks])

@api_bp.route('/logs', methods=['GET'])
@login_required
def logs():
    limit = request.args.get('limit', 100, type=int)
    logs = LogEntry.query.order_by(LogEntry.created_at.desc()).limit(limit).all()
    return jsonify([{
        'id': l.id,
        'level': l.level,
        'source': l.source,
        'message': l.message,
        'created_at': l.created_at.isoformat() if l.created_at else None,
    } for l in logs])

@api_bp.route('/monitor', methods=['GET'])
@login_required
def monitor():
    """Get current system metrics."""
    import psutil

    metrics = {
        'cpu_percent': psutil.cpu_percent(interval=0.5),
        'memory': {
            'total': psutil.virtual_memory().total,
            'available': psutil.virtual_memory().available,
            'percent': psutil.virtual_memory().percent,
        },
        'disk': {
            'total': psutil.disk_usage('/').total,
            'used': psutil.disk_usage('/').used,
            'percent': psutil.disk_usage('/').percent,
        },
        'network': {
            'bytes_sent': psutil.net_io_counters().bytes_sent,
            'bytes_recv': psutil.net_io_counters().bytes_recv,
        },
    }

    return jsonify(metrics)

@api_bp.route('/settings', methods=['GET', 'PUT'])
@login_required
def settings():
    if request.method == 'PUT':
        data = request.get_json()
        for key, value in data.items():
            setting = Setting.query.filter_by(key=key).first()
            if setting:
                setting.value = str(value)
            else:
                setting = Setting(key=key, value=str(value))
                db.session.add(setting)
        db.session.commit()
        return jsonify({'updated': True})

    settings = Setting.query.all()
    return jsonify({s.key: s.value for s in settings})

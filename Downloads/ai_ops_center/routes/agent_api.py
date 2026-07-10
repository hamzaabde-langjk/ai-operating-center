"""Agent-specific API routes."""
from flask import Blueprint, request, jsonify
from flask_login import login_required

agent_api_bp = Blueprint('agent_api', __name__)

@agent_api_bp.route('/', methods=['GET'])
@login_required
def list_agents():
    from run import center
    if not center.agent_factory:
        return jsonify({'error': 'System not initialized'}), 503
    return jsonify(center.agent_factory.list_agents())

@agent_api_bp.route('/<agent_id>/status', methods=['GET'])
@login_required
def agent_status(agent_id):
    from run import center
    if not center.agent_factory:
        return jsonify({'error': 'System not initialized'}), 503

    agent = center.agent_factory.get_agent(agent_id)
    if not agent:
        return jsonify({'error': 'Agent not found'}), 404

    return jsonify(agent.to_dict())

@agent_api_bp.route('/<agent_id>/tasks', methods=['POST'])
@login_required
def assign_task(agent_id):
    from run import center
    if not center.agent_factory:
        return jsonify({'error': 'System not initialized'}), 503

    data = request.get_json()
    from core.agent_base import AgentTask

    task = AgentTask(
        id=data.get('id', ''),
        title=data.get('title', ''),
        description=data.get('description', ''),
        priority=data.get('priority', 3),
    )

    return jsonify({'queued': True, 'agent_id': agent_id, 'task_title': task.title})

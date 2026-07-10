"""Main Flask application for AI Operations Center."""
import os
import logging
from datetime import datetime

from flask import Flask, render_template, request, jsonify, session
from flask_socketio import SocketIO, emit
from flask_login import LoginManager, login_required, current_user
from flask_limiter import Limiter
from flask_cors import CORS

from config import config_map
from database import db, migrate
from database.models import User, Agent, Task, Project, Event, LogEntry, Setting

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
    handlers=[
        logging.FileHandler('logs/app.log'),
        logging.StreamHandler(),
    ]
)
logger = logging.getLogger(__name__)

# Extensions
socketio = SocketIO()
login_manager = LoginManager()
limiter = Limiter(key_func=lambda: request.remote_addr)
cors = CORS()

def create_app(config_name='default'):
    """Application factory."""
    app = Flask(
        __name__,
        template_folder='templates',
        static_folder='static',
    )
    app.config.from_object(config_map[config_name])

    # Ensure directories exist
    os.makedirs(app.config['LOGS_DIR'], exist_ok=True)
    os.makedirs(app.config['PROJECTS_DIR'], exist_ok=True)
    os.makedirs(app.config['UPLOADS_DIR'], exist_ok=True)
    os.makedirs(app.config['PLUGINS_DIR'], exist_ok=True)

    # Initialize extensions
    db.init_app(app)
    migrate.init_app(app, db)
    socketio.init_app(
        app,
        async_mode=app.config['SOCKETIO_ASYNC_MODE'],
        cors_allowed_origins=app.config['SOCKETIO_CORS_ALLOWED_ORIGINS'],
    )
    login_manager.init_app(app)
    limiter.init_app(app)
    cors.init_app(app)

    login_manager.login_view = 'main.login'
    login_manager.login_message_category = 'info'

    @login_manager.user_loader
    def load_user(user_id):
        return User.query.get(int(user_id))

    # Register blueprints
    from routes.main import main_bp
    from routes.api import api_bp
    from routes.agent_api import agent_api_bp

    app.register_blueprint(main_bp)
    app.register_blueprint(api_bp, url_prefix='/api/v1')
    app.register_blueprint(agent_api_bp, url_prefix='/api/v1/agents')

    # Create tables
    with app.app_context():
        db.create_all()
        _seed_defaults()

    return app

def _seed_defaults():
    """Seed default data if not exists."""
    # Create default admin user
    if not User.query.filter_by(username='admin').first():
        from werkzeug.security import generate_password_hash
        admin = User(
            username='admin',
            email='admin@aiops.local',
            password_hash=generate_password_hash('admin'),
            role='admin',
        )
        db.session.add(admin)
        db.session.commit()

    # Create default settings
    defaults = {
        'default_temperature': '0.7',
        'default_max_tokens': '4096',
        'default_context_length': '8192',
        'monitor_interval': '5',
        'max_concurrent_tasks': '20',
    }
    for key, value in defaults.items():
        if not Setting.query.filter_by(key=key).first():
            db.session.add(Setting(key=key, value=value))
    db.session.commit()

app = create_app()

@socketio.on('connect')
def handle_connect():
    logger.info(f"Client connected: {request.sid}")
    emit('connected', {'status': 'connected', 'timestamp': datetime.utcnow().isoformat()})

@socketio.on('disconnect')
def handle_disconnect():
    logger.info(f"Client disconnected: {request.sid}")

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)

"""SQLAlchemy models for AI Operations Center."""
from datetime import datetime
from enum import Enum as PyEnum
from sqlalchemy import Enum, JSON, Text, Integer, String, DateTime, Boolean, ForeignKey, Float
from sqlalchemy.orm import relationship, declarative_base
from . import db

Base = declarative_base()

class TaskStatus(PyEnum):
    PENDING = 'pending'
    QUEUED = 'queued'
    RUNNING = 'running'
    COMPLETED = 'completed'
    FAILED = 'failed'
    RETRYING = 'retrying'
    CANCELLED = 'cancelled'

class Priority(PyEnum):
    CRITICAL = 1
    HIGH = 2
    MEDIUM = 3
    LOW = 4

class AgentStatus(PyEnum):
    IDLE = 'idle'
    BUSY = 'busy'
    OFFLINE = 'offline'
    ERROR = 'error'
    PAUSED = 'paused'

class UserRole(PyEnum):
    ADMIN = 'admin'
    OPERATOR = 'operator'
    VIEWER = 'viewer'

class Project(db.Model):
    __tablename__ = 'projects'
    id = db.Column(Integer, primary_key=True)
    name = db.Column(String(255), nullable=False, unique=True)
    description = db.Column(Text)
    goal = db.Column(Text)
    status = db.Column(String(50), default='active')
    created_at = db.Column(DateTime, default=datetime.utcnow)
    updated_at = db.Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    tasks = relationship('Task', back_populates='project', cascade='all, delete-orphan')
    milestones = relationship('Milestone', back_populates='project', cascade='all, delete-orphan')
    files = relationship('File', back_populates='project', cascade='all, delete-orphan')

class Agent(db.Model):
    __tablename__ = 'agents'
    id = db.Column(Integer, primary_key=True)
    name = db.Column(String(100), nullable=False, unique=True)
    role = db.Column(String(50), nullable=False)
    model = db.Column(String(100))
    status = db.Column(Enum(AgentStatus), default=AgentStatus.IDLE)
    memory = db.Column(JSON, default=dict)
    queue = db.Column(JSON, default=list)
    logs = db.Column(JSON, default=list)
    task_history = db.Column(JSON, default=list)
    statistics = db.Column(JSON, default=dict)
    created_at = db.Column(DateTime, default=datetime.utcnow)
    updated_at = db.Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    assigned_tasks = relationship('Task', back_populates='agent')

class Task(db.Model):
    __tablename__ = 'tasks'
    id = db.Column(Integer, primary_key=True)
    title = db.Column(String(255), nullable=False)
    description = db.Column(Text)
    priority = db.Column(Enum(Priority), default=Priority.MEDIUM)
    status = db.Column(Enum(TaskStatus), default=TaskStatus.PENDING)
    agent_id = db.Column(Integer, ForeignKey('agents.id'))
    project_id = db.Column(Integer, ForeignKey('projects.id'))
    logs = db.Column(JSON, default=list)
    result = db.Column(Text)
    error = db.Column(Text)
    created_at = db.Column(DateTime, default=datetime.utcnow)
    started_at = db.Column(DateTime)
    finished_at = db.Column(DateTime)
    retry_count = db.Column(Integer, default=0)
    dependencies = db.Column(JSON, default=list)
    metadata_info = db.Column(JSON, default=dict)

    agent = relationship('Agent', back_populates='assigned_tasks')
    project = relationship('Project', back_populates='tasks')

class Event(db.Model):
    __tablename__ = 'events'
    id = db.Column(Integer, primary_key=True)
    type = db.Column(String(50), nullable=False)
    source = db.Column(String(100))
    message = db.Column(Text)
    severity = db.Column(String(20), default='info')
    data = db.Column(JSON)
    created_at = db.Column(DateTime, default=datetime.utcnow)

class LogEntry(db.Model):
    __tablename__ = 'logs'
    id = db.Column(Integer, primary_key=True)
    level = db.Column(String(20), default='INFO')
    source = db.Column(String(100))
    message = db.Column(Text, nullable=False)
    data = db.Column(JSON)
    created_at = db.Column(DateTime, default=datetime.utcnow)

class Message(db.Model):
    __tablename__ = 'messages'
    id = db.Column(Integer, primary_key=True)
    sender = db.Column(String(100))
    receiver = db.Column(String(100))
    content = db.Column(Text)
    message_type = db.Column(String(50), default='chat')
    data = db.Column(JSON)
    created_at = db.Column(DateTime, default=datetime.utcnow)

class File(db.Model):
    __tablename__ = 'files'
    id = db.Column(Integer, primary_key=True)
    name = db.Column(String(255), nullable=False)
    path = db.Column(String(512))
    content = db.Column(Text)
    file_type = db.Column(String(50))
    project_id = db.Column(Integer, ForeignKey('projects.id'))
    created_at = db.Column(DateTime, default=datetime.utcnow)
    updated_at = db.Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    project = relationship('Project', back_populates='files')

class MemoryEntry(db.Model):
    __tablename__ = 'memories'
    id = db.Column(Integer, primary_key=True)
    memory_type = db.Column(String(50), nullable=False)  # short, long, shared, project
    key = db.Column(String(255))
    content = db.Column(Text)
    embedding = db.Column(JSON)
    agent_id = db.Column(Integer, ForeignKey('agents.id'), nullable=True)
    project_id = db.Column(Integer, ForeignKey('projects.id'), nullable=True)
    relevance_score = db.Column(Float, default=0.0)
    created_at = db.Column(DateTime, default=datetime.utcnow)
    accessed_at = db.Column(DateTime, default=datetime.utcnow)

class Setting(db.Model):
    __tablename__ = 'settings'
    id = db.Column(Integer, primary_key=True)
    key = db.Column(String(100), nullable=False, unique=True)
    value = db.Column(Text)
    category = db.Column(String(50), default='general')
    updated_at = db.Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

class Milestone(db.Model):
    __tablename__ = 'milestones'
    id = db.Column(Integer, primary_key=True)
    name = db.Column(String(255), nullable=False)
    description = db.Column(Text)
    status = db.Column(String(50), default='pending')
    project_id = db.Column(Integer, ForeignKey('projects.id'))
    due_date = db.Column(DateTime)
    completed_at = db.Column(DateTime)

    project = relationship('Project', back_populates='milestones')

class User(db.Model):
    __tablename__ = 'users'
    id = db.Column(Integer, primary_key=True)
    username = db.Column(String(100), nullable=False, unique=True)
    email = db.Column(String(255), nullable=False, unique=True)
    password_hash = db.Column(String(255), nullable=False)
    role = db.Column(Enum(UserRole), default=UserRole.VIEWER)
    is_active = db.Column(Boolean, default=True)
    last_login = db.Column(DateTime)
    created_at = db.Column(DateTime, default=datetime.utcnow)

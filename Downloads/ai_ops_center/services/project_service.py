"""Project management service."""
import logging
from typing import Any, Dict, List, Optional

from database import db
from database.models import Project, Milestone, File

logger = logging.getLogger(__name__)

class ProjectService:
    """Service for managing projects, milestones, and files."""

    def create_project(self, name: str, description: str = '', goal: str = '') -> Project:
        project = Project(
            name=name,
            description=description,
            goal=goal,
        )
        db.session.add(project)
        db.session.commit()
        logger.info(f"Created project: {name}")
        return project

    def get_project(self, project_id: int) -> Optional[Project]:
        return Project.query.get(project_id)

    def list_projects(self) -> List[Project]:
        return Project.query.order_by(Project.created_at.desc()).all()

    def create_milestone(self, project_id: int, name: str, description: str = '', due_date: Any = None) -> Optional[Milestone]:
        project = self.get_project(project_id)
        if not project:
            return None

        milestone = Milestone(
            name=name,
            description=description,
            project_id=project_id,
            due_date=due_date,
        )
        db.session.add(milestone)
        db.session.commit()
        return milestone

    def add_file(self, project_id: int, name: str, path: str = '', content: str = '', file_type: str = '') -> Optional[File]:
        project = self.get_project(project_id)
        if not project:
            return None

        file = File(
            name=name,
            path=path,
            content=content,
            file_type=file_type,
            project_id=project_id,
        )
        db.session.add(file)
        db.session.commit()
        return file

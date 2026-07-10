"""Main web routes."""
from flask import Blueprint, render_template, request, redirect, url_for, flash, jsonify
from flask_login import login_user, logout_user, login_required, current_user
from werkzeug.security import check_password_hash, generate_password_hash

from database import db
from database.models import User, UserRole

main_bp = Blueprint('main', __name__)

@main_bp.route('/')
def index():
    """Main dashboard."""
    return render_template('dashboard.html')

@main_bp.route('/login', methods=['GET', 'POST'])
def login():
    """User login."""
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')

        user = User.query.filter_by(username=username).first()
        if user and check_password_hash(user.password_hash, password):
            login_user(user)
            return redirect(url_for('main.index'))

        flash('Invalid credentials', 'danger')

    return render_template('login.html')

@main_bp.route('/logout')
@login_required
def logout():
    logout_user()
    return redirect(url_for('main.login'))

@main_bp.route('/projects')
@login_required
def projects():
    return render_template('projects.html')

@main_bp.route('/agents')
@login_required
def agents():
    return render_template('agents.html')

@main_bp.route('/memory')
@login_required
def memory():
    return render_template('memory.html')

@main_bp.route('/settings')
@login_required
def settings():
    return render_template('settings.html')

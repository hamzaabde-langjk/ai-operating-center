"""Terminal session service."""
import asyncio
import logging
import os
import pty
import select
import struct
import fcntl
import termios
import subprocess
from typing import Any, Dict, Optional

logger = logging.getLogger(__name__)

class TerminalService:
    """Manages interactive terminal sessions using pseudo-terminals."""

    def __init__(self):
        self.sessions: Dict[str, Dict] = {}

    def create_session(self, session_id: str, cols: int = 80, rows: int = 24) -> bool:
        """Create a new terminal session."""
        try:
            pid, fd = pty.fork()

            if pid == 0:
                # Child process
                env = os.environ.copy()
                env['TERM'] = 'xterm-256color'
                env['COLUMNS'] = str(cols)
                env['LINES'] = str(rows)
                os.execvpe('/bin/bash', ['bash', '-l'], env)
            else:
                # Parent process
                self.sessions[session_id] = {
                    'pid': pid,
                    'fd': fd,
                    'cols': cols,
                    'rows': rows,
                }

                # Set non-blocking
                fl = fcntl.fcntl(fd, fcntl.F_GETFL)
                fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)

                return True
        except Exception as e:
            logger.error(f"Failed to create terminal session: {e}")
            return False

    def resize(self, session_id: str, cols: int, rows: int) -> bool:
        """Resize terminal session."""
        session = self.sessions.get(session_id)
        if not session:
            return False

        try:
            session['cols'] = cols
            session['rows'] = rows
            size = struct.pack('HHHH', rows, cols, 0, 0)
            fcntl.ioctl(session['fd'], termios.TIOCSWINSZ, size)
            return True
        except Exception as e:
            logger.error(f"Resize error: {e}")
            return False

    def write(self, session_id: str, data: str) -> bool:
        """Write input to terminal."""
        session = self.sessions.get(session_id)
        if not session:
            return False

        try:
            os.write(session['fd'], data.encode())
            return True
        except Exception as e:
            logger.error(f"Write error: {e}")
            return False

    def read(self, session_id: str) -> Optional[str]:
        """Read output from terminal."""
        session = self.sessions.get(session_id)
        if not session:
            return None

        try:
            ready, _, _ = select.select([session['fd']], [], [], 0)
            if ready:
                data = os.read(session['fd'], 4096)
                return data.decode('utf-8', errors='replace')
            return ''
        except Exception as e:
            logger.error(f"Read error: {e}")
            return None

    def close(self, session_id: str) -> bool:
        """Close terminal session."""
        session = self.sessions.pop(session_id, None)
        if not session:
            return False

        try:
            os.close(session['fd'])
            os.kill(session['pid'], 9)
            return True
        except Exception as e:
            logger.error(f"Close error: {e}")
            return False

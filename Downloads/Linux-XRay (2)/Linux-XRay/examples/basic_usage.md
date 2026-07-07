# Basic Usage Examples

## Process Monitoring

```bash
# Monitor all process events
sudo xray --monitor --log-level=debug

# Monitor for 60 seconds and save to database
sudo xray --monitor --time=60 --database=my_session.db

# Export process events to JSON
xray --export=events.json --format=json
```

## Security Analysis

```bash
# Run security scan
sudo xray --monitor --analyze

# Check for specific threats
sudo xray --monitor --detect=privilege_escalation
```

## Timeline Replay

```bash
# Record a session
sudo xray --record=session.dat --time=300

# Replay the session
xray --play=session.dat
```

## GUI Mode

```bash
# Launch the 3D visualization
xray --gui

# Open specific view
xray --gui --view=security
```

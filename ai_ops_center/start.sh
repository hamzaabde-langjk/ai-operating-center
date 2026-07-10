#!/bin/bash


echo " Starting AI Operations Center with Cloudflare Tunnel..."

if ! command -v python3 &> /dev/null; then
    echo "❌ Python3 not found. Please install Python3 first."
    exit 1
fi

if ! command -v pip &> /dev/null; then
    echo "❌ pip not found. Installing pip..."
    sudo apt-get install python3-pip -y
fi
echo "📦 Installing Python dependencies..."
pip install -r requirements.txt
echo "🌐 Installing Cloudflare Tunnel..."
if ! command -v cloudflared &> /dev/null; then
    echo "Installing cloudflared manually..."
    wget -q https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64 -O cloudflared
    chmod +x cloudflared
    sudo mv cloudflared /usr/local/bin/
    echo "✅ Cloudflared installed successfully!"
else
    echo "✅ Cloudflared already installed."
fi
cloudflared --version

echo "🔄 Starting AI Operations Center in background..."
pkill -f "python3 run.py" 2>/dev/null
pkill -f "cloudflared tunnel" 2>/dev/null
nohup python3 run.py > logs/app.log 2>&1 &
APP_PID=$!
echo "✅ Application started with PID: $APP_PID"
echo "⏳ Waiting for application to start..."
sleep 5
echo "🌐 Starting Cloudflare Tunnel..."
cloudflared tunnel --url http://localhost:5000 &
CLOUDFLARE_PID=$!
echo "⏳ Waiting for Cloudflare Tunnel to establish connection..."
sleep 8

echo ""
echo "============================================"
echo "✅ AI Operations Center is now LIVE!"
echo "============================================"
echo ""
echo "🌍 Public URL:"
echo "   $(cloudflared tunnel --url http://localhost:5000 2>&1 | grep -o 'https://[a-zA-Z0-9-]*\.trycloudflare\.com' | head -1)"
echo ""
echo "📋 Local URL:"
echo "   http://localhost:5000"
echo ""
echo "🔑 Login Credentials:"
echo "   Username: admin"
echo "   Password: admin123"
echo ""
echo "============================================"
echo ""
echo "📊 Monitoring:"
echo "   - App PID: $APP_PID"
echo "   - Cloudflare PID: $CLOUDFLARE_PID"
echo "   - Logs: logs/app.log"
echo ""
echo "⚠️  Press Ctrl+C to stop all services"
echo "============================================"
wait $CLOUDFLARE_PID

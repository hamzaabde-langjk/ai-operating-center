#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' 


log() {
    echo -e "${BLUE}[$(date '+%Y-%m-%d %H:%M:%S')]${NC} $1"
}

success() {
    echo -e "${GREEN} $1${NC}"
}

error() {
    echo -e "${RED} $1${NC}"
}

warning() {
    echo -e "${YELLOW}  $1${NC}"
}

# ============================================
# دالة للتحقق من Ollama وإعادة المحاولة
# ============================================
wait_for_ollama() {
    local max_attempts=30
    local attempt=1
    local wait_time=2
    
    log "Waiting for Ollama to be ready..."
    
    while [ $attempt -le $max_attempts ]; do
        if curl -s http://localhost:11434 > /dev/null 2>&1; then
            success "Ollama is ready!"
            return 0
        fi
        
        # التحقق من أن عملية ollama لا تزال تعمل
        if ! pgrep -f "ollama serve" > /dev/null; then
            warning "Ollama process died. Restarting..."
            nohup ollama serve > logs/ollama.log 2>&1 &
            OLLAMA_PID=$!
        fi
        
        warning "Attempt $attempt/$max_attempts: Ollama not ready yet..."
        sleep $wait_time
        attempt=$((attempt + 1))
    done
    
    error "Ollama failed to start after $max_attempts attempts."
    error "Please check logs/ollama.log for details."
    error "You can also try running 'ollama serve' manually in another terminal."
    return 1
}

log "Checking Python installation..."
if ! command -v python3 &> /dev/null; then
    error "Python3 not found. Please install Python3 first."
    exit 1
fi
success "Python3 found: $(python3 --version)"

log "Checking pip installation..."
if ! command -v pip &> /dev/null; then
    warning "pip not found. Installing pip..."
    sudo apt-get install python3-pip -y
fi
success "pip found: $(pip --version)"

log "Installing Python dependencies..."
pip install -r requirements.txt
success "Python dependencies installed!"


log "Checking Ollama installation..."


install_ollama() {
    warning "Ollama not found. Installing Ollama..."

    if ! command -v zstd &> /dev/null; then
        warning "zstd not found. Installing zstd..."
        sudo apt-get update
        sudo apt-get install zstd -y
    fi
  
    curl -fsSL https://ollama.com/install.sh | sh
    
    if [ $? -eq 0 ]; then
        success "Ollama installed successfully!"
    else
        error "Failed to install Ollama. Please install manually."
        exit 1
    fi
}


if ! command -v ollama &> /dev/null; then
    install_ollama
else
    success "Ollama found: $(ollama --version)"
fi

log "Starting Ollama service..."


if ! curl -s http://localhost:11434 > /dev/null; then
    warning "Ollama not running. Starting Ollama in background..."
    mkdir -p logs
    nohup ollama serve > logs/ollama.log 2>&1 &
    OLLAMA_PID=$!
    log "Ollama started with PID: $OLLAMA_PID"
    
    # انتظار حتى يصبح Ollama جاهزاً
    if ! wait_for_ollama; then
        error "Cannot continue without Ollama."
        exit 1
    fi
else
    success "Ollama is already running"
    # التحقق من أن Ollama يستجيب بشكل صحيح
    if ! curl -s http://localhost:11434/api/tags > /dev/null 2>&1; then
        warning "Ollama is running but not responding properly. Restarting..."
        pkill -f "ollama serve" 2>/dev/null
        sleep 2
        nohup ollama serve > logs/ollama.log 2>&1 &
        OLLAMA_PID=$!
        if ! wait_for_ollama; then
            error "Cannot continue without Ollama."
            exit 1
        fi
    fi
fi


log "Checking required Ollama models..."


REQUIRED_MODELS=("qwen3:7b" "deepseek-coder:6.7b" "llama3:8b")
MISSING_MODELS=()


for model in "${REQUIRED_MODELS[@]}"; do
    if ! ollama list | grep -q "$model"; then
        MISSING_MODELS+=("$model")
    fi
done


if [ ${#MISSING_MODELS[@]} -ne 0 ]; then
    warning "Missing models: ${MISSING_MODELS[*]}"
    log "Downloading missing models..."
    
    for model in "${MISSING_MODELS[@]}"; do
        log "Downloading $model..."
        # إعادة محاولة تحميل النموذج إذا فشل
        retry_count=0
        max_retries=3
        while [ $retry_count -lt $max_retries ]; do
            if ollama pull "$model"; then
                success " $model downloaded successfully!"
                break
            else
                retry_count=$((retry_count + 1))
                if [ $retry_count -lt $max_retries ]; then
                    warning "Failed to download $model. Retrying ($retry_count/$max_retries)..."
                    sleep 3
                else
                    error " Failed to download $model after $max_retries attempts."
                fi
            fi
        done
    done
else
    success "All required models are available!"
fi


log "Available Ollama models:"
ollama list


log "Installing Cloudflare Tunnel..."
if ! command -v cloudflared &> /dev/null; then
    warning "Cloudflared not found. Installing manually..."
    wget -q https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64 -O cloudflared
    chmod +x cloudflared
    sudo mv cloudflared /usr/local/bin/
    success "Cloudflared installed successfully!"
else
    success "Cloudflared already installed: $(cloudflared --version)"
fi

mkdir -p logs

log "Starting AI Operations Center in background..."


pkill -f "python3 run.py" 2>/dev/null
pkill -f "cloudflared tunnel" 2>/dev/null

nohup python3 run.py > logs/app.log 2>&1 &
APP_PID=$!
success "Application started with PID: $APP_PID"

 
log "Waiting for application to start..."
sleep 5

log "Starting Cloudflare Tunnel..."
cloudflared tunnel --url http://localhost:5000 > logs/cloudflare.log 2>&1 &
CLOUDFLARE_PID=$!
success "Cloudflare Tunnel started with PID: $CLOUDFLARE_PID"


log "Waiting for Cloudflare Tunnel to establish connection..."
sleep 8


TUNNEL_URL=$(grep -o 'https://[a-zA-Z0-9-]*\.trycloudflare\.com' logs/cloudflare.log 2>/dev/null | head -1)
if [ -z "$TUNNEL_URL" ]; then
    TUNNEL_URL="Waiting for tunnel URL... Check logs/cloudflare.log"
fi

echo ""
echo "============================================"
echo -e "${GREEN} AI Operations Center is now LIVE!${NC}"
echo "============================================"
echo ""
echo -e "${BLUE} Public URL:${NC}"
echo -e "   ${GREEN}${TUNNEL_URL}${NC}"
echo ""
echo -e "${BLUE} Local URL:${NC}"
echo "   http://localhost:5000"
echo ""
echo -e "${BLUE} Login Credentials:${NC}"
echo "   Username: admin"
echo "   Password: admin123"
echo ""
echo -e "${BLUE} Ollama Status:${NC}"
echo "   - Service: $(curl -s http://localhost:11434 > /dev/null && echo ' Running' || echo ' Not running')"
echo "   - Models: $(ollama list | wc -l) models available"
echo ""
echo -e "${BLUE} Monitoring:${NC}"
echo "   - App PID: $APP_PID"
echo "   - Cloudflare PID: $CLOUDFLARE_PID"
echo "   - App Logs: logs/app.log"
echo "   - Ollama Logs: logs/ollama.log"
echo "   - Cloudflare Logs: logs/cloudflare.log"
echo ""
echo -e "${YELLOW}  Press Ctrl+C to stop all services${NC}"
echo "============================================"


echo "$TUNNEL_URL" > logs/tunnel_url.txt
echo "APP_PID=$APP_PID" > logs/services.pid
echo "CLOUDFLARE_PID=$CLOUDFLARE_PID" >> logs/services.pid


wait $CLOUDFLARE_PID

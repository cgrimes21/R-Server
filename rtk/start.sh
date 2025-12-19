#!/bin/bash
# RTK Server Startup Script
# =========================
# Starts all three servers: login, char, map
#
# Usage:
#   ./start.sh          - Start all servers
#   ./start.sh login    - Start login server only
#   ./start.sh char     - Start char server only
#   ./start.sh map      - Start map server only
#   ./start.sh stop     - Stop all servers
#   ./start.sh status   - Check if servers are running
#
# IMPORTANT: Run database setup first!
#   cd ../database && ./setup.sh init

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# PID files
LOGIN_PID="logs/login-server.pid"
CHAR_PID="logs/char-server.pid"
MAP_PID="logs/map-server.pid"

# Log files
LOGIN_LOG="logs/login-server.log"
CHAR_LOG="logs/char-server.log"
MAP_LOG="logs/map-server.log"
CRASH_LOG="logs/crash.log"

# Create logs directory if needed
mkdir -p logs

# Check MySQL connectivity
check_mysql() {
    # Load DB config if available
    local db_conf="$SCRIPT_DIR/../database/db.conf"
    if [[ -f "$db_conf" ]]; then
        source "$db_conf"
    else
        DB_USER="rtk"
        DB_PASS="50LM8U8Poq5uX2AZJVKs"
        DB_NAME="RTK"
        DB_HOST="127.0.0.1"
    fi

    if ! command -v mysql &> /dev/null; then
        echo -e "${RED}MySQL client not found. Is MySQL installed?${NC}"
        return 1
    fi

    if ! mysql -u "$DB_USER" -p"$DB_PASS" -h "$DB_HOST" "$DB_NAME" -e "SELECT 1" &> /dev/null; then
        echo -e "${RED}Cannot connect to MySQL database.${NC}"
        echo ""
        echo "Make sure:"
        echo "  1. MySQL is running: sudo service mysql start"
        echo "  2. Database is set up: cd ../database && ./setup.sh init"
        echo ""
        return 1
    fi
    return 0
}

start_server() {
    local name=$1
    local binary=$2
    local pidfile=$3
    local logfile=$4

    if [[ -f "$pidfile" ]] && kill -0 "$(cat "$pidfile")" 2>/dev/null; then
        echo -e "${YELLOW}$name is already running (PID: $(cat "$pidfile"))${NC}"
        return 1
    fi

    if [[ ! -f "$binary" ]]; then
        echo -e "${RED}$name binary not found: $binary${NC}"
        echo "Run 'make' in the rtk directory first."
        return 1
    fi

    # Clear old log and add startup header
    echo "========================================" > "$logfile"
    echo "[$name] Started at $(date)" >> "$logfile"
    echo "========================================" >> "$logfile"

    echo -e "${GREEN}Starting $name...${NC}"
    echo -e "${GREEN}  Logging to: $logfile${NC}"

    # Run with output redirected to log file
    ./"$binary" >> "$logfile" 2>&1 &
    local pid=$!
    echo $pid > "$pidfile"
    sleep 2

    if kill -0 "$pid" 2>/dev/null; then
        echo -e "${GREEN}$name started (PID: $pid)${NC}"
    else
        echo -e "${RED}$name failed to start - check $logfile${NC}"
        echo "" >> "$CRASH_LOG"
        echo "========================================" >> "$CRASH_LOG"
        echo "[$name] CRASH at $(date)" >> "$CRASH_LOG"
        echo "========================================" >> "$CRASH_LOG"
        tail -100 "$logfile" >> "$CRASH_LOG"
        rm -f "$pidfile"
        return 1
    fi
}

stop_server() {
    local name=$1
    local pidfile=$2

    if [[ ! -f "$pidfile" ]]; then
        echo -e "${YELLOW}$name is not running (no PID file)${NC}"
        return 0
    fi

    local pid=$(cat "$pidfile")
    if kill -0 "$pid" 2>/dev/null; then
        echo -e "${YELLOW}Stopping $name (PID: $pid)...${NC}"
        kill "$pid"
        sleep 1
        if kill -0 "$pid" 2>/dev/null; then
            echo -e "${RED}Force killing $name...${NC}"
            kill -9 "$pid"
        fi
        echo -e "${GREEN}$name stopped${NC}"
    else
        echo -e "${YELLOW}$name is not running${NC}"
    fi
    rm -f "$pidfile"
}

check_server() {
    local name=$1
    local pidfile=$2

    if [[ -f "$pidfile" ]] && kill -0 "$(cat "$pidfile")" 2>/dev/null; then
        echo -e "${GREEN}$name is running (PID: $(cat "$pidfile"))${NC}"
    else
        echo -e "${RED}$name is not running${NC}"
        [[ -f "$pidfile" ]] && rm -f "$pidfile"
    fi
}

cmd_start_all() {
    echo -e "${GREEN}=== Starting RTK Servers ===${NC}"
    echo ""

    # Check MySQL first
    echo "Checking MySQL connection..."
    if ! check_mysql; then
        exit 1
    fi
    echo -e "${GREEN}MySQL connection OK${NC}"
    echo ""

    start_server "Login Server" "login-server" "$LOGIN_PID" "$LOGIN_LOG"
    echo "Waiting for Login Server to initialize..."
    sleep 5
    start_server "Char Server" "char-server" "$CHAR_PID" "$CHAR_LOG"
    echo "Waiting for Char Server to initialize..."
    sleep 5
    start_server "Map Server" "map-server" "$MAP_PID" "$MAP_LOG"
    echo ""
    echo -e "${GREEN}=== All servers started ===${NC}"
    echo ""
    echo "Log files:"
    echo "  Login:  $LOGIN_LOG"
    echo "  Char:   $CHAR_LOG"
    echo "  Map:    $MAP_LOG"
    echo "  Crash:  $CRASH_LOG"
    echo ""
    echo "Use './start.sh status' to check, './start.sh stop' to stop"
    echo "Use './start.sh tail' to watch map server logs live"
}

cmd_stop_all() {
    echo -e "${YELLOW}=== Stopping RTK Servers ===${NC}"
    echo ""
    stop_server "Map Server" "$MAP_PID"
    stop_server "Char Server" "$CHAR_PID"
    stop_server "Login Server" "$LOGIN_PID"
    echo ""
    echo -e "${GREEN}=== All servers stopped ===${NC}"
}

cmd_status() {
    echo -e "${GREEN}=== RTK Server Status ===${NC}"
    echo ""
    check_server "Login Server" "$LOGIN_PID"
    check_server "Char Server" "$CHAR_PID"
    check_server "Map Server" "$MAP_PID"
}

cmd_tail() {
    local server=${1:-map}
    local logfile
    case "$server" in
        login) logfile="$LOGIN_LOG" ;;
        char)  logfile="$CHAR_LOG" ;;
        map)   logfile="$MAP_LOG" ;;
        crash) logfile="$CRASH_LOG" ;;
        *)     logfile="$MAP_LOG" ;;
    esac
    echo -e "${GREEN}Tailing $logfile (Ctrl+C to stop)${NC}"
    tail -f "$logfile"
}

cmd_logs() {
    echo -e "${GREEN}=== Recent Errors/Warnings ===${NC}"
    echo ""
    for log in "$LOGIN_LOG" "$CHAR_LOG" "$MAP_LOG"; do
        if [[ -f "$log" ]]; then
            local errors=$(grep -i -E '\[Error\]|\[Warning\]|\[Fatal|SEGFAULT|Segmentation|crash' "$log" 2>/dev/null | tail -20)
            if [[ -n "$errors" ]]; then
                echo -e "${YELLOW}--- $log ---${NC}"
                echo "$errors"
                echo ""
            fi
        fi
    done
    if [[ -f "$CRASH_LOG" ]]; then
        echo -e "${RED}--- $CRASH_LOG ---${NC}"
        tail -50 "$CRASH_LOG"
    fi
}

cmd_help() {
    echo "RTK Server Startup Script"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  (none)    Start all servers"
    echo "  login     Start login server only"
    echo "  char      Start char server only"
    echo "  map       Start map server only"
    echo "  stop      Stop all servers"
    echo "  status    Check server status"
    echo "  tail [s]  Tail logs (login/char/map/crash, default: map)"
    echo "  logs      Show recent errors from all logs"
    echo "  help      Show this help"
}

case "${1:-start}" in
    start)
        cmd_start_all
        ;;
    login)
        start_server "Login Server" "login-server" "$LOGIN_PID" "$LOGIN_LOG"
        ;;
    char)
        start_server "Char Server" "char-server" "$CHAR_PID" "$CHAR_LOG"
        ;;
    map)
        start_server "Map Server" "map-server" "$MAP_PID" "$MAP_LOG"
        ;;
    stop)
        cmd_stop_all
        ;;
    status)
        cmd_status
        ;;
    tail)
        cmd_tail "$2"
        ;;
    logs)
        cmd_logs
        ;;
    help|--help|-h)
        cmd_help
        ;;
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        cmd_help
        exit 1
        ;;
esac

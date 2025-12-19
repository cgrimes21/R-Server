#!/bin/bash
# RTK Database Setup Script
# =========================
# Reads credentials from db.conf and sets up the database.
#
# Usage:
#   ./setup.sh init      - Fresh install (creates database, user, tables)
#   ./setup.sh migrate   - Run all pending migrations
#   ./setup.sh sync      - Sync db.conf credentials to char.conf
#   ./setup.sh status    - Show migration status
#   ./setup.sh reset     - Drop and recreate database (DESTRUCTIVE!)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/db.conf"
CHAR_CONF="$SCRIPT_DIR/../rtk/conf/char.conf"
SAVE_CONF="$SCRIPT_DIR/../rtk/conf/save.conf"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Load configuration
load_config() {
    if [[ ! -f "$CONFIG_FILE" ]]; then
        echo -e "${RED}Error: db.conf not found at $CONFIG_FILE${NC}"
        exit 1
    fi

    # Source the config file (it's bash-compatible)
    source "$CONFIG_FILE"

    # Validate required variables
    for var in DB_NAME DB_USER DB_PASS DB_HOST DB_PORT; do
        if [[ -z "${!var}" ]]; then
            echo -e "${RED}Error: $var not set in db.conf${NC}"
            exit 1
        fi
    done
}

# Run MySQL as root
mysql_root() {
    sudo mysql "$@"
}

# Run MySQL as the rtk user
mysql_user() {
    mysql -u "$DB_USER" -p"$DB_PASS" -h "$DB_HOST" -P "$DB_PORT" "$@"
}

# Initialize database from scratch
cmd_init() {
    echo -e "${GREEN}=== RTK Database Initialization ===${NC}"
    echo "Database: $DB_NAME"
    echo "User: $DB_USER"
    echo "Host: $DB_HOST:$DB_PORT"
    echo ""

    # Generate and run the init SQL
    echo -e "${YELLOW}Creating database and user...${NC}"

    mysql_root <<EOF
-- Create database
CREATE DATABASE IF NOT EXISTS \`$DB_NAME\` CHARACTER SET latin1;
USE \`$DB_NAME\`;

-- Drop existing users if they exist
DROP USER IF EXISTS '$DB_USER'@'%';
DROP USER IF EXISTS '$DB_USER'@'localhost';

-- Create users
CREATE USER '$DB_USER'@'%' IDENTIFIED BY '$DB_PASS';
CREATE USER '$DB_USER'@'localhost' IDENTIFIED BY '$DB_PASS';

-- Grant privileges
GRANT ALL PRIVILEGES ON $DB_NAME.* TO '$DB_USER'@'%';
GRANT ALL PRIVILEGES ON $DB_NAME.* TO '$DB_USER'@'localhost';
FLUSH PRIVILEGES;

SELECT 'Database and user created successfully' AS Status;
EOF

    echo -e "${GREEN}Database and user created.${NC}"
    echo ""

    # Run the table creation script
    echo -e "${YELLOW}Running migration scripts...${NC}"
    cd "$SCRIPT_DIR/scripts"

    for script in [0-9][0-9]_*.sql; do
        if [[ -f "$script" ]]; then
            echo "  Running $script..."
            mysql_root "$DB_NAME" < "$script"
        fi
    done

    echo ""
    echo -e "${GREEN}=== Initialization Complete ===${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Run './setup.sh sync' to update char.conf"
    echo "  2. Start the server"
}

# Run pending migrations only
cmd_migrate() {
    echo -e "${GREEN}=== Running Migrations ===${NC}"

    cd "$SCRIPT_DIR/scripts"

    for script in [0-9][0-9]_*.sql; do
        if [[ -f "$script" ]]; then
            echo "  Running $script..."
            mysql_user "$DB_NAME" < "$script" 2>/dev/null || mysql_root "$DB_NAME" < "$script"
        fi
    done

    echo -e "${GREEN}Migrations complete.${NC}"
}

# Sync credentials to config files
cmd_sync() {
    echo -e "${GREEN}=== Syncing credentials to server configs ===${NC}"

    # List of config files to update
    local configs=("$CHAR_CONF" "$SAVE_CONF")
    local config_names=("char.conf" "save.conf")

    for i in "${!configs[@]}"; do
        local conf="${configs[$i]}"
        local name="${config_names[$i]}"

        if [[ ! -f "$conf" ]]; then
            echo -e "${YELLOW}Warning: $name not found at $conf - skipping${NC}"
            continue
        fi

        # Create backup
        cp "$conf" "$conf.bak"
        echo "  Backup created: $name.bak"

        # Update the values using sed
        sed -i "s/^sql_ip:.*/sql_ip: $DB_HOST/" "$conf"
        sed -i "s/^sql_port:.*/sql_port: $DB_PORT/" "$conf"
        sed -i "s/^sql_id:.*/sql_id: $DB_USER/" "$conf"
        sed -i "s/^sql_pw:.*/sql_pw: $DB_PASS/" "$conf"
        sed -i "s/^sql_db:.*/sql_db: $DB_NAME/" "$conf"

        echo -e "${GREEN}  $name updated${NC}"
    done

    echo ""
    echo "Credentials synced:"
    echo "  sql_ip: $DB_HOST"
    echo "  sql_port: $DB_PORT"
    echo "  sql_id: $DB_USER"
    echo "  sql_pw: $DB_PASS"
    echo "  sql_db: $DB_NAME"
}

# Show migration status
cmd_status() {
    echo -e "${GREEN}=== Migration Status ===${NC}"
    echo ""

    mysql_user "$DB_NAME" -e "SELECT Script, Timestamp FROM MigrationHistory ORDER BY Id;" 2>/dev/null || \
    mysql_root "$DB_NAME" -e "SELECT Script, Timestamp FROM MigrationHistory ORDER BY Id;" 2>/dev/null || \
    echo -e "${YELLOW}MigrationHistory table not found. Run './setup.sh init' first.${NC}"
}

# Reset database (destructive!)
cmd_reset() {
    echo -e "${RED}=== WARNING: This will DELETE all data! ===${NC}"
    read -p "Are you sure? Type 'yes' to confirm: " confirm

    if [[ "$confirm" != "yes" ]]; then
        echo "Aborted."
        exit 0
    fi

    echo -e "${YELLOW}Dropping database...${NC}"
    mysql_root -e "DROP DATABASE IF EXISTS \`$DB_NAME\`;"

    echo -e "${YELLOW}Dropping users...${NC}"
    mysql_root -e "DROP USER IF EXISTS '$DB_USER'@'%';"
    mysql_root -e "DROP USER IF EXISTS '$DB_USER'@'localhost';"

    echo -e "${GREEN}Database reset complete. Run './setup.sh init' to recreate.${NC}"
}

# Show help
cmd_help() {
    echo "RTK Database Setup Script"
    echo ""
    echo "Usage: $0 <command>"
    echo ""
    echo "Commands:"
    echo "  init      Fresh install (creates database, user, runs all migrations)"
    echo "  migrate   Run all pending migration scripts"
    echo "  sync      Sync db.conf credentials to server config files"
    echo "  status    Show which migrations have been applied"
    echo "  reset     Drop and recreate database (DESTRUCTIVE!)"
    echo "  help      Show this help message"
    echo ""
    echo "Configuration:"
    echo "  Master config:  $CONFIG_FILE"
    echo "  Syncs to:       rtk/conf/char.conf"
    echo "                  rtk/conf/save.conf"
    echo ""
    echo "Examples:"
    echo "  ./setup.sh init          # First-time setup"
    echo "  ./setup.sh sync          # After changing db.conf"
    echo "  ./setup.sh status        # Check migration status"
}

# Main
load_config

case "${1:-help}" in
    init)
        cmd_init
        ;;
    migrate)
        cmd_migrate
        ;;
    sync)
        cmd_sync
        ;;
    status)
        cmd_status
        ;;
    reset)
        cmd_reset
        ;;
    help|--help|-h)
        cmd_help
        ;;
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        echo ""
        cmd_help
        exit 1
        ;;
esac

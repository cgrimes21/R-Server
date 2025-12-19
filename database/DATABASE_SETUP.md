# RTK Server Database Setup Guide

This guide explains how to set up the MySQL database for the RTK Server.

## Quick Start (Recommended)

### Step 1: Install MySQL

**WSL (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install mysql-server
sudo service mysql start
```

**Windows (standalone):**
1. Download from https://dev.mysql.com/downloads/mysql/
2. Run installer, set root password when prompted
3. Start MySQL service

**Docker:**
```bash
docker run --name rtk-mysql -e MYSQL_ROOT_PASSWORD=root -p 3306:3306 -d mysql:8.0
```

### Step 2: Run Setup

#### Using Default Credentials

If you're fine with the default credentials, just run:

```bash
cd database
./setup.sh init
```

That's it! The defaults are already configured everywhere.

**Default credentials:**
- Database: `RTK`
- User: `rtk`
- Password: `50LM8U8Poq5uX2AZJVKs`
- Host: `127.0.0.1:3306`

### Step 3: Start the Servers

```bash
cd rtk
./start.sh
```

See [Starting the Servers](#starting-the-servers) for more options.

#### Using Custom Credentials

```bash
# 1. Edit credentials
nano database/db.conf

# 2. Run the setup script
cd database
./setup.sh init

# 3. Sync credentials to server configs
./setup.sh sync
```

The setup script will:
- Create the RTK database
- Create the rtk user with your configured password
- Run all migration scripts (including game data)
- `sync` updates char.conf and save.conf to match db.conf

---

## Configuration

All database credentials are stored in `database/db.conf`:

```bash
# Database name
DB_NAME=RTK

# MySQL user for the game server
DB_USER=rtk

# Password - CHANGE THIS IN PRODUCTION!
DB_PASS=50LM8U8Poq5uX2AZJVKs

# MySQL host (use 127.0.0.1 for WSL compatibility)
DB_HOST=127.0.0.1

# MySQL port
DB_PORT=3306
```

After editing db.conf, run `./setup.sh sync` to update `rtk/conf/char.conf`.

---

## Setup Script Commands

| Command | Description |
|---------|-------------|
| `./setup.sh init` | Fresh install - creates database, user, runs all migrations |
| `./setup.sh migrate` | Run any pending migration scripts |
| `./setup.sh sync` | Sync db.conf credentials to char.conf |
| `./setup.sh status` | Show which migrations have been applied |
| `./setup.sh reset` | Drop and recreate database (DESTRUCTIVE!) |

---

## Manual Setup (Alternative)

If you prefer not to use setup.sh:

### Option 1: Empty Tables (Development)

```bash
sudo mysql < database/rtk_init.sql
```

### Option 2: Full Game Data

```bash
cd database/scripts
sudo mysql < 01_CreateRtkDatabaseAndUser.sql
sudo mysql RTK < 02_CreateTables.sql
sudo mysql RTK < 03_FixErrors.sql
# ... additional migrations as needed
```

**Note:** Manual setup uses hardcoded credentials. Edit the SQL files or use setup.sh for custom credentials.

---

## Prerequisites

- MySQL Server 5.7+ or MariaDB 10.3+
- Root/admin access to MySQL
- Bash shell (WSL, Linux, or Git Bash on Windows)

### Step 1: Install MySQL

#### WSL (Ubuntu/Debian)

```bash
# Update packages
sudo apt update

# Install MySQL Server
sudo apt install mysql-server

# Start MySQL service
sudo service mysql start

# (Optional) Enable auto-start
sudo systemctl enable mysql
```

#### Windows

**Option A: Standalone MySQL**
1. Download MySQL Community Server from https://dev.mysql.com/downloads/mysql/
2. Run the installer and follow the wizard
3. Set a root password when prompted

**Option B: XAMPP (includes Apache, PHP, MySQL)**
1. Download XAMPP from https://www.apachefriends.org/
2. Install and start the MySQL module from XAMPP Control Panel

**Option C: Docker**
```bash
docker run --name rtk-mysql -e MYSQL_ROOT_PASSWORD=rootpass -p 3306:3306 -d mysql:8.0
```

### Step 2: Verify MySQL is Running

```bash
# WSL/Linux
sudo service mysql status

# Or try connecting
mysql -u root -p -e "SELECT VERSION();"
```

### Step 3: Run Setup Script

```bash
cd /path/to/RTK-Server/database
./setup.sh init
./setup.sh sync
```

### Step 4: Verify the Setup

```bash
# Check migration status
./setup.sh status

# Or connect manually
mysql -u rtk -p RTK -e "SHOW TABLES;"
```

Expected output should show 40+ tables including:
- `Accounts`, `Character`, `Inventory`, `Equipment`
- `Maps`, `Mobs`, `NPCs0`, `Items`, `Spells`
- And more...

---

## Database Configuration

Database credentials are managed in `database/db.conf`. The setup script syncs these to `rtk/conf/char.conf`.

**To change credentials:**
```bash
# 1. Edit the master config
nano database/db.conf

# 2. Sync to char.conf
./setup.sh sync

# 3. If database already exists, update the MySQL user password:
sudo mysql -e "ALTER USER 'rtk'@'%' IDENTIFIED BY 'new_password';"
sudo mysql -e "ALTER USER 'rtk'@'localhost' IDENTIFIED BY 'new_password';"
```

**Files that use database credentials:**
- `database/db.conf` - Master config (edit this one)
- `rtk/conf/char.conf` - Character server (synced by setup.sh)
- `rtk/conf/save.conf` - Save server (synced by setup.sh)

---

## Tables Overview

### Player Data
| Table | Description |
|-------|-------------|
| `Accounts` | User accounts (email, password, ban status) |
| `Character` | Character data (name, stats, location, etc.) |
| `Inventory` | Items in character inventory |
| `Equipment` | Currently equipped items |
| `Banks` | Bank storage items |
| `SpellBook` | Learned spells |
| `Legends` | Legend marks (achievements) |
| `Friends` | Friend lists |
| `Mail` | In-game mail |
| `Parcels` | Item parcels/gifts |

### Game World
| Table | Description |
|-------|-------------|
| `Maps` | Map definitions |
| `Warps` | Warp points between maps |
| `Paths` | Named locations on maps |
| `NPCs0` | NPC definitions and locations |
| `Mobs` | Monster definitions |
| `Spawns0`, `Spawns1` | Monster spawn points |

### Items & Spells
| Table | Description |
|-------|-------------|
| `Items` | Item definitions |
| `ItemSets` | Set bonus definitions |
| `Recipes` | Crafting recipes |
| `Spells` | Spell definitions |

### Registries (Key-Value Storage)
| Table | Description |
|-------|-------------|
| `Registry` | Per-character integer storage |
| `RegistryString` | Per-character string storage |
| `QuestRegistry` | Quest progress tracking |
| `GameRegistry0` | Global game state |
| `MapRegistry` | Per-map state |
| `NPCRegistry` | Per-NPC state |

---

## Starting the Servers

Once the database is set up, start the game servers:

```bash
cd rtk
./start.sh
```

### Server Commands

| Command | Description |
|---------|-------------|
| `./start.sh` | Start all servers (login, char, map) |
| `./start.sh stop` | Stop all servers |
| `./start.sh status` | Check which servers are running |
| `./start.sh login` | Start login server only |
| `./start.sh char` | Start char server only |
| `./start.sh map` | Start map server only |

### Server Start Order

The servers must start in this order:
1. **Login Server** - Handles authentication
2. **Char Server** - Handles character data
3. **Map Server** - Handles gameplay

The start script handles this automatically.

### Checking Logs

Server logs are in `rtk/logs/`:
- `login-server.pid` - Login server process ID
- `char-server.pid` - Char server process ID
- `map-server.pid` - Map server process ID

---

## Troubleshooting

### "Access denied for user 'root'@'localhost'"

WSL often has MySQL configured without a root password:
```bash
# Try without password
sudo mysql

# Or reset the root password
sudo mysql
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'newpassword';
FLUSH PRIVILEGES;
```

### "Can't connect to MySQL server"

1. Check if MySQL is running:
   ```bash
   sudo service mysql status
   ```

2. Start it if not:
   ```bash
   sudo service mysql start
   ```

3. Check it's listening on the right port:
   ```bash
   sudo netstat -tlnp | grep 3306
   ```

### "Table doesn't exist" errors

The game data tables (Maps, Items, Mobs, etc.) need to be populated. If you used `rtk_init.sql`, the tables are empty. You need to either:
1. Run the full migration scripts, OR
2. Import game data separately

### "Unknown column" or schema mismatch

The database schema may have changed. Run any missing migration scripts from `database/scripts/` in chronological order.

### Server connects but shows "MySQL Error"

Check `rtk/conf/char.conf` matches your database credentials:
```ini
sql_ip: 127.0.0.1      # Use 127.0.0.1, not 'localhost' for WSL
sql_port: 3306
sql_id: rtk
sql_pw: 50LM8U8Poq5uX2AZJVKs
sql_db: RTK
```

---

## Backup & Restore

### Create Backup

```bash
# Full backup with data
mysqldump -u rtk -p'50LM8U8Poq5uX2AZJVKs' RTK > rtk_backup_$(date +%Y%m%d).sql

# Schema only (no data)
mysqldump -u rtk -p'50LM8U8Poq5uX2AZJVKs' --no-data RTK > rtk_schema.sql
```

### Restore from Backup

```bash
mysql -u rtk -p'50LM8U8Poq5uX2AZJVKs' RTK < rtk_backup_20251212.sql
```

---

## Migration Scripts Reference

The `database/scripts/` folder contains incremental migrations. See [MIGRATIONS.md](MIGRATIONS.md) for detailed documentation on each script.

**Required (run in order):**
| Script | Description |
|--------|-------------|
| `01_CreateRtkDatabaseAndUser.sql` | Creates database, user, MigrationHistory |
| `02_CreateTables.sql` | Creates all tables with initial game data |
| `03_FixErrors.sql` | Fixes initial data issues |

**Optional (balance, content, fixes):**
| Script | Description |
|--------|-------------|
| `04_ReduceMobVitaAndExp.sql` | Balance: reduces mob HP/XP |
| `05_DoNotRequireAccountPassword.sql` | Auth: allow empty passwords |
| `06_AddTigerSentries.sql` | Content: new Tiger mobs |
| `07-21_*.sql` | Various fixes, balance, and content |

Each script tracks itself in `MigrationHistory` to prevent duplicate runs.

---

## Security Notes

**For Production:**

1. **Change the default password!**
   ```sql
   ALTER USER 'rtk'@'%' IDENTIFIED BY 'your_secure_password';
   ALTER USER 'rtk'@'localhost' IDENTIFIED BY 'your_secure_password';
   FLUSH PRIVILEGES;
   ```
   Then update `char.conf` to match.

2. **Restrict user access:**
   ```sql
   -- Remove wildcard access
   DROP USER 'rtk'@'%';
   -- Keep only localhost
   ```

3. **Use environment variables** for passwords instead of hardcoding in config files.

4. **Regular backups** - Use the backup script in `database/backup.sh`

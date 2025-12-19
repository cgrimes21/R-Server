# Building RetroTK Server

This document describes how to build the RetroTK Server components.

## Prerequisites

### Linux/WSL

Install required dependencies:

```bash
sudo apt update
sudo apt install build-essential liblua5.1-dev libmysqlclient-dev zlib1g-dev
```

### Required Libraries

| Library | Package | Purpose |
|---------|---------|---------|
| Lua 5.1 | liblua5.1-dev | Scripting engine |
| MySQL Client | libmysqlclient-dev | Database connectivity |
| zlib | zlib1g-dev | Compression |
| GCC | build-essential | C compiler |

---

## Build Commands

All commands should be run from the `rtk/` directory.

```bash
cd rtk
```

### Build All Servers

```bash
make all
```

### Build Individual Servers

```bash
make map      # Build map-server only
make login    # Build login-server only
make char     # Build char-server only
```

### Clean Build

```bash
make clean    # Remove all build artifacts
make all      # Rebuild everything
```

---

## Build Output

Successful builds produce the following binaries:

| Binary | Size | Location |
|--------|------|----------|
| map-server | ~2MB | rtk/ |
| char-server | ~528KB | rtk/ |
| login-server | ~464KB | rtk/ |

---

## Build Configuration

### Compiler Flags

The Makefile uses the following flags:

```makefile
CFLAGS = -DDEBUG -g3 -fno-stack-protector -ffast-math -Wall -Wno-sign-compare \
         -DFD_SETSIZE=1024 -DNO_MEMMGR -DLOGGING_ENABLED -DUSE_MYSQL
```

| Flag | Purpose |
|------|---------|
| `-DDEBUG` | Enable debug mode |
| `-g3` | Maximum debug symbols |
| `-fno-stack-protector` | Disable stack protection (for performance) |
| `-ffast-math` | Aggressive floating-point optimizations |
| `-Wall` | Enable all warnings |
| `-Wno-sign-compare` | Disable signed/unsigned comparison warnings |
| `-DFD_SETSIZE=1024` | Maximum file descriptors |
| `-DNO_MEMMGR` | Disable custom memory manager |
| `-DLOGGING_ENABLED` | Enable logging |
| `-DUSE_MYSQL` | Use MySQL backend |

### Include Paths

```makefile
-I../common -I/usr/include/mysql -I/usr/include/lua5.1
```

---

## Makefile Structure

### Object File Groups

```makefile
COMMON_OBJ   # Shared utilities (db, socket, timer, etc.)
MAP_OBJ      # Map server core
CLIF_OBJ     # Client interface modules (client_*.c)
SL_OBJ       # Lua scripting modules (sl_*.c)
CHAR_OBJ     # Character server
LOGIN_OBJ    # Login server
```

---

## Syntax Checking

To check syntax without full compilation:

```bash
cd rtk/src/map
gcc -c -DDEBUG -g3 -fno-stack-protector -ffast-math -Wall -Wno-sign-compare \
    -DFD_SETSIZE=1024 -DNO_MEMMGR -DLOGGING_ENABLED -DUSE_MYSQL \
    -I../common -I/usr/include/mysql -I/usr/include/lua5.1 \
    -fsyntax-only <file.c>
```

---

## Troubleshooting

### MySQL 8.0 Compatibility

If you encounter `my_bool` errors:

```c
// In db_mysql.c, replace:
my_bool reconnect = true;

// With:
bool reconnect = true;
```

### crypt() Name Collision

If you get linker errors about `crypt()`:

The functions have been renamed to `rtk_crypt()` and `rtk_crypt2()` to avoid collision with libc's crypt().

### Missing Headers

Ensure all required dev packages are installed:

```bash
sudo apt install liblua5.1-dev libmysqlclient-dev zlib1g-dev
```

### Linking Errors

If you get undefined reference errors, ensure the Makefile includes all object files. The object groups are:

- `COMMON_OBJ` - Common utilities
- `CLIF_OBJ` - Client interface modules
- `SL_OBJ` - Scripting modules

---

## WSL-Specific Notes

When building on Windows Subsystem for Linux:

1. Use WSL Ubuntu (recommended: 20.04 or later)
2. Install packages via apt as shown above
3. Run make from within WSL, not Windows cmd
4. Binaries are Linux ELF format (run in WSL or deploy to Linux server)

---

## Running the Servers

After building, run servers in order:

```bash
# Terminal 1
./login-server

# Terminal 2
./char-server

# Terminal 3
./map-server
```

Configuration files must be present in `conf/` directory before starting.

---

## Related Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - Codebase structure
- [REFACTORING.md](REFACTORING.md) - Refactoring details

/*
 * server_config.c - Centralized Server Configuration Implementation
 *
 * Part of RTK Server refactoring - Phase 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "server_config.h"
#include "showmsg.h"

/* Global configuration instance */
rtk_config_t g_config;

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Trim whitespace from both ends of a string
 */
static char* trim(char* str) {
    char* end;

    /* Trim leading space */
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    /* Write new null terminator */
    end[1] = '\0';

    return str;
}

/**
 * Parse a comma-separated list of bytes into an array
 * Format: "1,2,3,4,5" or "0x01,0x02,0x03"
 */
static int parse_byte_array(const char* str, unsigned char* array, int max_count) {
    char buffer[1024];
    char* token;
    int count = 0;

    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    token = strtok(buffer, ",");
    while (token != NULL && count < max_count) {
        char* trimmed = trim(token);
        int value;

        /* Support both decimal and hex format */
        if (strncmp(trimmed, "0x", 2) == 0 || strncmp(trimmed, "0X", 2) == 0) {
            value = (int)strtol(trimmed, NULL, 16);
        } else {
            value = atoi(trimmed);
        }

        if (value >= 0 && value <= 255) {
            array[count++] = (unsigned char)value;
        }

        token = strtok(NULL, ",");
    }

    return count;
}

/* ============================================================================
 * Configuration Initialization
 * ============================================================================ */

void config_init_defaults(void) {
    memset(&g_config, 0, sizeof(rtk_config_t));

    /* Database defaults */
    strcpy(g_config.database.ip, "127.0.0.1");
    g_config.database.port = 3306;
    strcpy(g_config.database.user, "rtk");
    strcpy(g_config.database.password, "");
    strcpy(g_config.database.database, "rtk");

    /* Network defaults */
    strcpy(g_config.network.map_ip, "127.0.0.1");
    g_config.network.map_port = 2001;
    strcpy(g_config.network.login_ip, "127.0.0.1");
    g_config.network.login_port = 2000;
    strcpy(g_config.network.char_ip, "127.0.0.1");
    g_config.network.char_port = 2002;

    /* Rate defaults */
    g_config.rates.experience = 1;
    g_config.rates.drop = 1;
    g_config.rates.gold = 1;
    g_config.rates.crafting = 1;

    /* Server defaults */
    g_config.server.server_id = 0;
    strcpy(g_config.server.server_name, "RetroTK");
    g_config.server.save_interval = 60;
    g_config.server.dump_unknown_packets = 0;
    g_config.server.attack_delay = 1000;
    g_config.server.log_error = 1;
    g_config.server.log_dump_packet = 0;
    g_config.server.log_command = 1;

    /* Crypto defaults - these match the original hardcoded values */
    /* Client key 2 packets */
    {
        unsigned char defaults[] = { 6, 8, 9, 10, 15, 19, 23, 26, 28, 41, 45, 46, 50, 57 };
        memcpy(g_config.crypto.client_key2, defaults, sizeof(defaults));
        g_config.crypto.client_key2_count = sizeof(defaults);
    }

    /* Server key 2 packets */
    {
        unsigned char defaults[] = { 4, 7, 8, 11, 12, 19, 23, 24, 51, 54, 57, 64, 99 };
        memcpy(g_config.crypto.server_key2, defaults, sizeof(defaults));
        g_config.crypto.server_key2_count = sizeof(defaults);
    }

    /* Server key 1 packets */
    {
        unsigned char defaults[] = { 2, 3, 10, 64, 68, 94, 96, 98, 102, 111 };
        memcpy(g_config.crypto.server_key1_packets, defaults, sizeof(defaults));
        g_config.crypto.server_key1_count = sizeof(defaults);
    }

    /* Client key 1 packets */
    {
        unsigned char defaults[] = { 2, 3, 4, 11, 21, 38, 58, 66, 67, 75, 80, 87, 98, 113, 115, 123 };
        memcpy(g_config.crypto.client_key1_packets, defaults, sizeof(defaults));
        g_config.crypto.client_key1_count = sizeof(defaults);
    }

    /* Log defaults */
    strcpy(g_config.logs.map_log, "logs/map.log");
    strcpy(g_config.logs.dump_log, "logs/map_dump.log");
    strcpy(g_config.logs.error_log, "logs/error.log");
    strcpy(g_config.logs.command_log, "logs/command.log");

    g_config.loaded = 0;
}

/* ============================================================================
 * Configuration Loading
 * ============================================================================ */

int config_load(const char* filename) {
    FILE* fp;
    char line[1024];
    char key[256];
    char value[768];

    if (filename == NULL) {
        ShowError("config_load: NULL filename\n");
        return -1;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        ShowError("config_load: Cannot open file '%s'\n", filename);
        return -1;
    }

    ShowStatus("Loading configuration from '%s'\n", filename);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char* p;
        char* trimmed;

        /* Remove newline */
        p = strchr(line, '\n');
        if (p) *p = '\0';
        p = strchr(line, '\r');
        if (p) *p = '\0';

        /* Skip comments and empty lines */
        trimmed = trim(line);
        if (trimmed[0] == '/' || trimmed[0] == '#' || trimmed[0] == '\0') {
            continue;
        }

        /* Parse key: value */
        p = strchr(trimmed, ':');
        if (p == NULL) {
            continue;
        }

        *p = '\0';
        strncpy(key, trim(trimmed), sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        strncpy(value, trim(p + 1), sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0';

        /* Database settings */
        if (strcasecmp(key, "sql_ip") == 0) {
            strncpy(g_config.database.ip, value, sizeof(g_config.database.ip) - 1);
        }
        else if (strcasecmp(key, "sql_port") == 0) {
            g_config.database.port = (uint16_t)atoi(value);
        }
        else if (strcasecmp(key, "sql_id") == 0) {
            strncpy(g_config.database.user, value, sizeof(g_config.database.user) - 1);
        }
        else if (strcasecmp(key, "sql_pw") == 0) {
            strncpy(g_config.database.password, value, sizeof(g_config.database.password) - 1);
        }
        else if (strcasecmp(key, "sql_db") == 0) {
            strncpy(g_config.database.database, value, sizeof(g_config.database.database) - 1);
        }
        /* Network settings */
        else if (strcasecmp(key, "map_ip") == 0) {
            strncpy(g_config.network.map_ip, value, sizeof(g_config.network.map_ip) - 1);
        }
        else if (strcasecmp(key, "map_port") == 0) {
            g_config.network.map_port = (uint16_t)atoi(value);
        }
        else if (strcasecmp(key, "loginip") == 0) {
            strncpy(g_config.network.login_ip, value, sizeof(g_config.network.login_ip) - 1);
        }
        else if (strcasecmp(key, "loginport") == 0) {
            g_config.network.login_port = (uint16_t)atoi(value);
        }
        /* Rate settings */
        else if (strcasecmp(key, "xprate") == 0) {
            g_config.rates.experience = atoi(value);
        }
        else if (strcasecmp(key, "droprate") == 0) {
            g_config.rates.drop = atoi(value);
        }
        else if (strcasecmp(key, "goldrate") == 0) {
            g_config.rates.gold = atoi(value);
        }
        else if (strcasecmp(key, "craftrate") == 0) {
            g_config.rates.crafting = atoi(value);
        }
        /* Server settings */
        else if (strcasecmp(key, "ServerId") == 0) {
            g_config.server.server_id = atoi(value);
        }
        else if (strcasecmp(key, "ServerName") == 0) {
            strncpy(g_config.server.server_name, value, sizeof(g_config.server.server_name) - 1);
        }
        else if (strcasecmp(key, "save_time") == 0) {
            g_config.server.save_interval = atoi(value);
        }
        else if (strcasecmp(key, "dump_save") == 0) {
            g_config.server.dump_unknown_packets = atoi(value);
        }
        else if (strcasecmp(key, "attack_delay") == 0) {
            g_config.server.attack_delay = atoi(value);
        }
        else if (strcasecmp(key, "log_error") == 0) {
            g_config.server.log_error = atoi(value);
        }
        else if (strcasecmp(key, "log_dump_packet") == 0) {
            g_config.server.log_dump_packet = atoi(value);
        }
        else if (strcasecmp(key, "log_command") == 0) {
            g_config.server.log_command = atoi(value);
        }
        /* Log paths */
        else if (strcasecmp(key, "map_log") == 0) {
            strncpy(g_config.logs.map_log, value, sizeof(g_config.logs.map_log) - 1);
        }
        else if (strcasecmp(key, "dump_log") == 0) {
            strncpy(g_config.logs.dump_log, value, sizeof(g_config.logs.dump_log) - 1);
        }
        /* Import other config files */
        else if (strcasecmp(key, "import") == 0) {
            config_load_import(value);
        }
        else if (strcasecmp(key, "crypto_config") == 0) {
            config_load_crypto(value);
        }
    }

    fclose(fp);
    g_config.loaded = 1;
    ShowStatus("Configuration loaded successfully\n");
    return 0;
}

int config_load_import(const char* filename) {
    /* Recursive call to load additional config */
    return config_load(filename);
}

int config_load_crypto(const char* filename) {
    FILE* fp;
    char line[1024];
    char key[256];
    char value[768];

    if (filename == NULL) {
        return -1;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        ShowWarning("config_load_crypto: Cannot open file '%s', using defaults\n", filename);
        return -1;
    }

    ShowStatus("Loading crypto configuration from '%s'\n", filename);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char* p;
        char* trimmed;

        /* Remove newline */
        p = strchr(line, '\n');
        if (p) *p = '\0';
        p = strchr(line, '\r');
        if (p) *p = '\0';

        /* Skip comments and empty lines */
        trimmed = trim(line);
        if (trimmed[0] == '/' || trimmed[0] == '#' || trimmed[0] == '\0') {
            continue;
        }

        /* Parse key: value */
        p = strchr(trimmed, ':');
        if (p == NULL) {
            continue;
        }

        *p = '\0';
        strncpy(key, trim(trimmed), sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        strncpy(value, trim(p + 1), sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0';

        /* Parse crypto settings */
        if (strcasecmp(key, "client_key2") == 0) {
            g_config.crypto.client_key2_count = parse_byte_array(value,
                g_config.crypto.client_key2, MAX_PACKET_KEYS);
        }
        else if (strcasecmp(key, "server_key2") == 0) {
            g_config.crypto.server_key2_count = parse_byte_array(value,
                g_config.crypto.server_key2, MAX_PACKET_KEYS);
        }
        else if (strcasecmp(key, "server_key1_packets") == 0) {
            g_config.crypto.server_key1_count = parse_byte_array(value,
                g_config.crypto.server_key1_packets, MAX_PACKET_KEYS);
        }
        else if (strcasecmp(key, "client_key1_packets") == 0) {
            g_config.crypto.client_key1_count = parse_byte_array(value,
                g_config.crypto.client_key1_packets, MAX_PACKET_KEYS);
        }
    }

    fclose(fp);
    return 0;
}

/* ============================================================================
 * Configuration Accessors
 * ============================================================================ */

const database_config_t* config_get_database(void) {
    return &g_config.database;
}

const network_config_t* config_get_network(void) {
    return &g_config.network;
}

const rate_config_t* config_get_rates(void) {
    return &g_config.rates;
}

const server_config_t* config_get_server(void) {
    return &g_config.server;
}

const crypto_config_t* config_get_crypto(void) {
    return &g_config.crypto;
}

const log_config_t* config_get_logs(void) {
    return &g_config.logs;
}

/* ============================================================================
 * Crypto Helper Functions
 * ============================================================================ */

int config_is_client_key2_packet(int packet_id) {
    int i;
    for (i = 0; i < g_config.crypto.client_key2_count; i++) {
        if (g_config.crypto.client_key2[i] == packet_id) {
            return 1;
        }
    }
    return 0;
}

int config_is_server_key2_packet(int packet_id) {
    int i;
    for (i = 0; i < g_config.crypto.server_key2_count; i++) {
        if (g_config.crypto.server_key2[i] == packet_id) {
            return 1;
        }
    }
    return 0;
}

int config_is_client_key1_packet(int packet_id) {
    int i;
    for (i = 0; i < g_config.crypto.client_key1_count; i++) {
        if (g_config.crypto.client_key1_packets[i] == packet_id) {
            return 1;
        }
    }
    return 0;
}

int config_is_server_key1_packet(int packet_id) {
    int i;
    for (i = 0; i < g_config.crypto.server_key1_count; i++) {
        if (g_config.crypto.server_key1_packets[i] == packet_id) {
            return 1;
        }
    }
    return 0;
}

/* ============================================================================
 * Debug Functions
 * ============================================================================ */

void config_dump(void) {
    int i;

    ShowInfo("=== Configuration Dump ===\n");

    ShowInfo("Database:\n");
    ShowInfo("  IP: %s\n", g_config.database.ip);
    ShowInfo("  Port: %d\n", g_config.database.port);
    ShowInfo("  User: %s\n", g_config.database.user);
    ShowInfo("  Database: %s\n", g_config.database.database);

    ShowInfo("Network:\n");
    ShowInfo("  Map IP: %s:%d\n", g_config.network.map_ip, g_config.network.map_port);
    ShowInfo("  Login IP: %s:%d\n", g_config.network.login_ip, g_config.network.login_port);

    ShowInfo("Rates:\n");
    ShowInfo("  Experience: %d\n", g_config.rates.experience);
    ShowInfo("  Drop: %d\n", g_config.rates.drop);
    ShowInfo("  Gold: %d\n", g_config.rates.gold);

    ShowInfo("Server:\n");
    ShowInfo("  ID: %d\n", g_config.server.server_id);
    ShowInfo("  Name: %s\n", g_config.server.server_name);
    ShowInfo("  Save Interval: %d seconds\n", g_config.server.save_interval);

    ShowInfo("Crypto:\n");
    ShowInfo("  Client Key2 Packets (%d): ", g_config.crypto.client_key2_count);
    for (i = 0; i < g_config.crypto.client_key2_count; i++) {
        ShowMessage("%d ", g_config.crypto.client_key2[i]);
    }
    ShowMessage("\n");

    ShowInfo("  Server Key2 Packets (%d): ", g_config.crypto.server_key2_count);
    for (i = 0; i < g_config.crypto.server_key2_count; i++) {
        ShowMessage("%d ", g_config.crypto.server_key2[i]);
    }
    ShowMessage("\n");

    ShowInfo("=== End Configuration ===\n");
}

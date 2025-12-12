/*
 * server_config.h - Centralized Server Configuration
 *
 * Provides structured configuration management for the RTK server.
 * Groups related settings together and provides type-safe access.
 *
 * Part of RTK Server refactoring - Phase 3.3
 */

#ifndef _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include <stdint.h>

/* Maximum sizes for configuration strings */
#define CONFIG_STRING_MAX 256
#define CONFIG_PATH_MAX 512
#define CONFIG_KEY_MAX 32

/* ============================================================================
 * Database Configuration
 * ============================================================================ */
typedef struct database_config {
    char ip[CONFIG_STRING_MAX];
    uint16_t port;
    char user[CONFIG_STRING_MAX];
    char password[CONFIG_STRING_MAX];
    char database[CONFIG_STRING_MAX];
} database_config_t;

/* ============================================================================
 * Network Configuration
 * ============================================================================ */
typedef struct network_config {
    char map_ip[CONFIG_STRING_MAX];
    uint16_t map_port;
    char login_ip[CONFIG_STRING_MAX];
    uint16_t login_port;
    char char_ip[CONFIG_STRING_MAX];
    uint16_t char_port;
    char log_ip[CONFIG_STRING_MAX];
    uint16_t log_port;
} network_config_t;

/* ============================================================================
 * Game Rate Configuration
 * ============================================================================ */
typedef struct rate_config {
    int experience;     /* XP multiplier */
    int drop;           /* Drop rate multiplier */
    int gold;           /* Gold drop multiplier */
    int crafting;       /* Crafting success rate modifier */
} rate_config_t;

/* ============================================================================
 * Server Configuration
 * ============================================================================ */
typedef struct server_config {
    int server_id;
    char server_name[CONFIG_STRING_MAX];
    int save_interval;          /* Auto-save interval in seconds */
    int dump_unknown_packets;   /* Whether to dump unknown packets */
    int attack_delay;           /* Attack time delay in ms */
    int log_error;
    int log_dump_packet;
    int log_command;
} server_config_t;

/* ============================================================================
 * Encryption Configuration (Packet Keys)
 * ============================================================================ */
#define MAX_PACKET_KEYS 32

typedef struct crypto_config {
    /* Client encryption key 2 - packets that use key2 encryption */
    unsigned char client_key2[MAX_PACKET_KEYS];
    int client_key2_count;

    /* Server encryption key 2 - packets that use key2 encryption */
    unsigned char server_key2[MAX_PACKET_KEYS];
    int server_key2_count;

    /* Server key1 packets - packets that use key1 encryption */
    unsigned char server_key1_packets[MAX_PACKET_KEYS];
    int server_key1_count;

    /* Client key1 packets - packets that use key1 encryption */
    unsigned char client_key1_packets[MAX_PACKET_KEYS];
    int client_key1_count;
} crypto_config_t;

/* ============================================================================
 * Log File Configuration
 * ============================================================================ */
typedef struct log_config {
    char map_log[CONFIG_PATH_MAX];
    char dump_log[CONFIG_PATH_MAX];
    char error_log[CONFIG_PATH_MAX];
    char command_log[CONFIG_PATH_MAX];
} log_config_t;

/* ============================================================================
 * Master Configuration Structure
 * ============================================================================ */
typedef struct rtk_config {
    database_config_t database;
    network_config_t network;
    rate_config_t rates;
    server_config_t server;
    crypto_config_t crypto;
    log_config_t logs;
    int loaded;  /* Flag indicating if config is loaded */
} rtk_config_t;

/* ============================================================================
 * Global Configuration Instance
 * ============================================================================ */
extern rtk_config_t g_config;

/* ============================================================================
 * Configuration API
 * ============================================================================ */

/**
 * Initialize configuration with default values
 */
void config_init_defaults(void);

/**
 * Load configuration from file
 * @param filename Path to configuration file
 * @return 0 on success, -1 on error
 */
int config_load(const char* filename);

/**
 * Load additional configuration file (import)
 * @param filename Path to configuration file
 * @return 0 on success, -1 on error
 */
int config_load_import(const char* filename);

/**
 * Load crypto configuration from file
 * @param filename Path to crypto configuration file
 * @return 0 on success, -1 on error
 */
int config_load_crypto(const char* filename);

/**
 * Get database configuration
 * @return Pointer to database configuration
 */
const database_config_t* config_get_database(void);

/**
 * Get network configuration
 * @return Pointer to network configuration
 */
const network_config_t* config_get_network(void);

/**
 * Get rate configuration
 * @return Pointer to rate configuration
 */
const rate_config_t* config_get_rates(void);

/**
 * Get server configuration
 * @return Pointer to server configuration
 */
const server_config_t* config_get_server(void);

/**
 * Get crypto configuration
 * @return Pointer to crypto configuration
 */
const crypto_config_t* config_get_crypto(void);

/**
 * Get log configuration
 * @return Pointer to log configuration
 */
const log_config_t* config_get_logs(void);

/**
 * Check if a packet ID uses key2 encryption (client->server)
 * @param packet_id The packet ID to check
 * @return 1 if uses key2, 0 otherwise
 */
int config_is_client_key2_packet(int packet_id);

/**
 * Check if a packet ID uses key2 encryption (server->client)
 * @param packet_id The packet ID to check
 * @return 1 if uses key2, 0 otherwise
 */
int config_is_server_key2_packet(int packet_id);

/**
 * Check if a packet ID uses key1 encryption (client->server)
 * @param packet_id The packet ID to check
 * @return 1 if uses key1, 0 if uses key2
 */
int config_is_client_key1_packet(int packet_id);

/**
 * Check if a packet ID uses key1 encryption (server->client)
 * @param packet_id The packet ID to check
 * @return 1 if uses key1, 0 if uses key2
 */
int config_is_server_key1_packet(int packet_id);

/**
 * Dump current configuration to log
 */
void config_dump(void);

#endif /* _SERVER_CONFIG_H_ */

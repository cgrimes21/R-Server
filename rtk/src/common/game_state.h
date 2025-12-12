/**
 * game_state.h - Centralized game runtime state management
 *
 * This module consolidates scattered global variables into structured types,
 * making server state explicit and improving maintainability.
 *
 * Part of RTK Server Phase 3.2 refactoring.
 */

#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

#include <stdint.h>

/* Forward declarations */
struct Sql;
struct map_data;

/*============================================================================
 * Constants
 *============================================================================*/

#define STATE_STRING_MAX 64
#define MAX_AUTH_FIFO 4096

/*============================================================================
 * Network Connection State
 * Tracks active connections to other servers
 *============================================================================*/

typedef struct network_state {
    /* Character server connection */
    int char_fd;
    uint32_t char_ip;
    uint16_t char_port;
    char char_ip_str[16];
    char char_id[32];
    char char_pw[32];

    /* Map server listening */
    int map_fd;
    uint32_t map_ip;
    uint16_t map_port;
    char map_ip_str[16];

    /* Log server connection */
    int log_fd;
    uint32_t log_ip;
    uint16_t log_port;
    char log_ip_str[16];

} network_state_t;

/*============================================================================
 * Server Identity State
 * Information identifying this server instance
 *============================================================================*/

typedef struct server_identity {
    int id;                         /* Server ID */
    char name[STATE_STRING_MAX];    /* Server name */
} server_identity_t;

/*============================================================================
 * Game Time State
 * Tracks in-game time and real-time for cron jobs
 *============================================================================*/

typedef struct time_state {
    /* Current game time */
    int cur_time;
    int old_time;

    /* Game calendar */
    int cur_year;
    int cur_day;
    int cur_season;

    /* Real time tracking for cron */
    int old_hour;
    int old_minute;
    int cronjob_timer;

} time_state_t;

/*============================================================================
 * Spawn State
 * Tracks mob and NPC spawning counters
 *============================================================================*/

typedef struct spawn_state {
    /* Mob spawning */
    uint32_t mob_spawn_start;
    uint32_t mob_spawn_max;
    uint32_t mob_spawn_current;

    /* One-time mob spawning */
    uint32_t mob_onetime_start;
    uint32_t mob_onetime_max;
    uint32_t mob_onetime_current;

    /* NPC spawning */
    uint32_t npc_id_current;

    /* Timing */
    uint32_t min_timer;

} spawn_state_t;

/*============================================================================
 * Statistics State
 * Server-wide statistics and counters
 *============================================================================*/

typedef struct stats_state {
    /* User counts */
    int online_count;
    int peak_online;

    /* Block list tracking */
    int bl_list_count;

    /* Object tracking */
    int object_count;

} stats_state_t;

/*============================================================================
 * Database State
 * Database connection handles (runtime state, not config)
 *============================================================================*/

typedef struct db_state {
    struct Sql* sql_handle;
    int connected;
} db_state_t;

/*============================================================================
 * Main Game State Structure
 * Consolidates all runtime state into a single structure
 *============================================================================*/

typedef struct game_state {
    /* Subsystem states */
    network_state_t network;
    server_identity_t server;
    time_state_t time;
    spawn_state_t spawn;
    stats_state_t stats;
    db_state_t database;

    /* Initialization flags */
    int initialized;
    int running;

} game_state_t;

/*============================================================================
 * Global State Instance
 *============================================================================*/

extern game_state_t g_state;

/*============================================================================
 * Initialization Functions
 *============================================================================*/

/**
 * Initialize game state with default values
 */
void state_init_defaults(void);

/**
 * Check if game state is initialized
 * @return 1 if initialized, 0 otherwise
 */
int state_is_initialized(void);

/*============================================================================
 * Network State Accessors
 *============================================================================*/

/**
 * Set character server connection info
 */
void state_set_char_connection(int fd, uint32_t ip, uint16_t port);

/**
 * Set map server listening info
 */
void state_set_map_listen(int fd, uint32_t ip, uint16_t port);

/**
 * Set log server connection info
 */
void state_set_log_connection(int fd, uint32_t ip, uint16_t port);

/**
 * Get character server file descriptor
 */
int state_get_char_fd(void);

/**
 * Get map server file descriptor
 */
int state_get_map_fd(void);

/**
 * Get log server file descriptor
 */
int state_get_log_fd(void);

/*============================================================================
 * Server Identity Accessors
 *============================================================================*/

/**
 * Set server identity
 */
void state_set_server_identity(int id, const char* name);

/**
 * Get server ID
 */
int state_get_server_id(void);

/**
 * Get server name
 */
const char* state_get_server_name(void);

/*============================================================================
 * Time State Accessors
 *============================================================================*/

/**
 * Update current game time
 */
void state_set_time(int cur_time);

/**
 * Get current game time
 */
int state_get_time(void);

/**
 * Update game calendar
 */
void state_set_calendar(int year, int day, int season);

/**
 * Update cron tracking
 */
void state_set_cron_time(int hour, int minute);

/*============================================================================
 * Spawn State Accessors
 *============================================================================*/

/**
 * Get next mob spawn ID
 */
uint32_t state_next_mob_id(void);

/**
 * Get next one-time mob ID
 */
uint32_t state_next_onetime_id(void);

/**
 * Get next NPC ID
 */
uint32_t state_next_npc_id(void);

/**
 * Reset spawn counters (for server restart)
 */
void state_reset_spawn_counters(void);

/*============================================================================
 * Statistics Accessors
 *============================================================================*/

/**
 * Update online player count
 */
void state_set_online_count(int count);

/**
 * Increment online count
 */
void state_inc_online_count(void);

/**
 * Decrement online count
 */
void state_dec_online_count(void);

/**
 * Get current online count
 */
int state_get_online_count(void);

/**
 * Get peak online count
 */
int state_get_peak_online(void);

/*============================================================================
 * Database State Accessors
 *============================================================================*/

/**
 * Set database connection handle
 */
void state_set_db_handle(struct Sql* handle);

/**
 * Get database connection handle
 */
struct Sql* state_get_db_handle(void);

/**
 * Check if database is connected
 */
int state_db_connected(void);

#endif /* _GAME_STATE_H_ */

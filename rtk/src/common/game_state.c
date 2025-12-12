/**
 * game_state.c - Centralized game runtime state management implementation
 *
 * Part of RTK Server Phase 3.2 refactoring.
 */

#include <string.h>
#include "game_state.h"
#include "showmsg.h"

/*============================================================================
 * Global State Instance
 *============================================================================*/

game_state_t g_state = {0};

/*============================================================================
 * Default Values
 *============================================================================*/

/* Default spawn ranges matching original code */
#define DEFAULT_MOB_SPAWN_START     1073741823
#define DEFAULT_MOB_SPAWN_MAX       100000000
#define DEFAULT_MOB_ONETIME_START   1173741823
#define DEFAULT_MOB_ONETIME_MAX     100000000
#define DEFAULT_NPC_ID_START        3221225472
#define DEFAULT_MIN_TIMER           1000

/*============================================================================
 * Initialization Functions
 *============================================================================*/

void state_init_defaults(void)
{
    /* Clear all state */
    memset(&g_state, 0, sizeof(game_state_t));

    /* Initialize network state */
    g_state.network.char_fd = -1;
    g_state.network.map_fd = -1;
    g_state.network.log_fd = -1;

    /* Initialize spawn state with defaults */
    g_state.spawn.mob_spawn_start = DEFAULT_MOB_SPAWN_START;
    g_state.spawn.mob_spawn_max = DEFAULT_MOB_SPAWN_MAX;
    g_state.spawn.mob_spawn_current = DEFAULT_MOB_SPAWN_START;

    g_state.spawn.mob_onetime_start = DEFAULT_MOB_ONETIME_START;
    g_state.spawn.mob_onetime_max = DEFAULT_MOB_ONETIME_MAX;
    g_state.spawn.mob_onetime_current = DEFAULT_MOB_ONETIME_START;

    g_state.spawn.npc_id_current = DEFAULT_NPC_ID_START;
    g_state.spawn.min_timer = DEFAULT_MIN_TIMER;

    /* Mark as initialized */
    g_state.initialized = 1;
    g_state.running = 0;

    ShowStatus("Game state initialized with defaults.\n");
}

int state_is_initialized(void)
{
    return g_state.initialized;
}

/*============================================================================
 * Network State Accessors
 *============================================================================*/

void state_set_char_connection(int fd, uint32_t ip, uint16_t port)
{
    g_state.network.char_fd = fd;
    g_state.network.char_ip = ip;
    g_state.network.char_port = port;
}

void state_set_map_listen(int fd, uint32_t ip, uint16_t port)
{
    g_state.network.map_fd = fd;
    g_state.network.map_ip = ip;
    g_state.network.map_port = port;
}

void state_set_log_connection(int fd, uint32_t ip, uint16_t port)
{
    g_state.network.log_fd = fd;
    g_state.network.log_ip = ip;
    g_state.network.log_port = port;
}

int state_get_char_fd(void)
{
    return g_state.network.char_fd;
}

int state_get_map_fd(void)
{
    return g_state.network.map_fd;
}

int state_get_log_fd(void)
{
    return g_state.network.log_fd;
}

/*============================================================================
 * Server Identity Accessors
 *============================================================================*/

void state_set_server_identity(int id, const char* name)
{
    g_state.server.id = id;
    if (name) {
        strncpy(g_state.server.name, name, STATE_STRING_MAX - 1);
        g_state.server.name[STATE_STRING_MAX - 1] = '\0';
    }
}

int state_get_server_id(void)
{
    return g_state.server.id;
}

const char* state_get_server_name(void)
{
    return g_state.server.name;
}

/*============================================================================
 * Time State Accessors
 *============================================================================*/

void state_set_time(int cur_time)
{
    g_state.time.old_time = g_state.time.cur_time;
    g_state.time.cur_time = cur_time;
}

int state_get_time(void)
{
    return g_state.time.cur_time;
}

void state_set_calendar(int year, int day, int season)
{
    g_state.time.cur_year = year;
    g_state.time.cur_day = day;
    g_state.time.cur_season = season;
}

void state_set_cron_time(int hour, int minute)
{
    g_state.time.old_hour = hour;
    g_state.time.old_minute = minute;
}

/*============================================================================
 * Spawn State Accessors
 *============================================================================*/

uint32_t state_next_mob_id(void)
{
    uint32_t id = g_state.spawn.mob_spawn_current;

    if (g_state.spawn.mob_spawn_current < g_state.spawn.mob_spawn_start + g_state.spawn.mob_spawn_max) {
        g_state.spawn.mob_spawn_current++;
    } else {
        /* Wrap around - should not happen in normal operation */
        ShowWarning("Mob spawn ID wrapped around!\n");
        g_state.spawn.mob_spawn_current = g_state.spawn.mob_spawn_start;
    }

    return id;
}

uint32_t state_next_onetime_id(void)
{
    uint32_t id = g_state.spawn.mob_onetime_current;

    if (g_state.spawn.mob_onetime_current < g_state.spawn.mob_onetime_start + g_state.spawn.mob_onetime_max) {
        g_state.spawn.mob_onetime_current++;
    } else {
        /* Wrap around - should not happen in normal operation */
        ShowWarning("One-time mob spawn ID wrapped around!\n");
        g_state.spawn.mob_onetime_current = g_state.spawn.mob_onetime_start;
    }

    return id;
}

uint32_t state_next_npc_id(void)
{
    return g_state.spawn.npc_id_current++;
}

void state_reset_spawn_counters(void)
{
    g_state.spawn.mob_spawn_current = g_state.spawn.mob_spawn_start;
    g_state.spawn.mob_onetime_current = g_state.spawn.mob_onetime_start;
    g_state.spawn.npc_id_current = DEFAULT_NPC_ID_START;

    ShowStatus("Spawn counters reset.\n");
}

/*============================================================================
 * Statistics Accessors
 *============================================================================*/

void state_set_online_count(int count)
{
    g_state.stats.online_count = count;
    if (count > g_state.stats.peak_online) {
        g_state.stats.peak_online = count;
    }
}

void state_inc_online_count(void)
{
    g_state.stats.online_count++;
    if (g_state.stats.online_count > g_state.stats.peak_online) {
        g_state.stats.peak_online = g_state.stats.online_count;
    }
}

void state_dec_online_count(void)
{
    if (g_state.stats.online_count > 0) {
        g_state.stats.online_count--;
    }
}

int state_get_online_count(void)
{
    return g_state.stats.online_count;
}

int state_get_peak_online(void)
{
    return g_state.stats.peak_online;
}

/*============================================================================
 * Database State Accessors
 *============================================================================*/

void state_set_db_handle(struct Sql* handle)
{
    g_state.database.sql_handle = handle;
    g_state.database.connected = (handle != NULL) ? 1 : 0;
}

struct Sql* state_get_db_handle(void)
{
    return g_state.database.sql_handle;
}

int state_db_connected(void)
{
    return g_state.database.connected;
}

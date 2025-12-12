/**
 * rtk_error.h - Standardized error handling utilities
 *
 * This module provides standardized error codes and helper macros for
 * consistent error handling across the RTK Server codebase.
 *
 * Part of RTK Server Phase 4.1 refactoring.
 *
 * Usage Guidelines:
 * -----------------
 * 1. Functions should return rtk_result_t for operations that can fail
 * 2. Use RTK_SUCCESS for successful operations
 * 3. Use specific error codes for failures
 * 4. Use RTK_CHECK_* macros for common validation patterns
 * 5. Use RTK_RETURN_* macros for consistent error reporting
 */

#ifndef _RTK_ERROR_H_
#define _RTK_ERROR_H_

#include <stddef.h>  /* For NULL */
#include "showmsg.h"

/*============================================================================
 * Result Type
 *============================================================================*/

/**
 * Standard result type for RTK functions
 * Positive values indicate success, negative indicate errors
 */
typedef int rtk_result_t;

/*============================================================================
 * Standard Return Codes
 *============================================================================*/

/* Success codes (>= 0) */
#define RTK_SUCCESS             0   /* Operation completed successfully */
#define RTK_SUCCESS_PARTIAL     1   /* Operation partially completed */
#define RTK_SUCCESS_NOOP        2   /* Operation succeeded but nothing done */

/* General errors (-1 to -99) */
#define RTK_ERROR               -1  /* Generic error */
#define RTK_ERROR_INVALID_PARAM -2  /* Invalid parameter passed */
#define RTK_ERROR_NULL_POINTER  -3  /* Null pointer where not expected */
#define RTK_ERROR_OUT_OF_RANGE  -4  /* Value out of valid range */
#define RTK_ERROR_NOT_FOUND     -5  /* Requested item not found */
#define RTK_ERROR_ALREADY_EXISTS -6 /* Item already exists */
#define RTK_ERROR_NOT_INIT      -7  /* System not initialized */
#define RTK_ERROR_TIMEOUT       -8  /* Operation timed out */

/* Memory errors (-100 to -199) */
#define RTK_ERROR_NO_MEMORY     -100 /* Memory allocation failed */
#define RTK_ERROR_BUFFER_FULL   -101 /* Buffer is full */
#define RTK_ERROR_BUFFER_EMPTY  -102 /* Buffer is empty */

/* Network errors (-200 to -299) */
#define RTK_ERROR_NETWORK       -200 /* Generic network error */
#define RTK_ERROR_DISCONNECT    -201 /* Connection disconnected */
#define RTK_ERROR_SEND_FAILED   -202 /* Failed to send data */
#define RTK_ERROR_RECV_FAILED   -203 /* Failed to receive data */
#define RTK_ERROR_INVALID_PACKET -204 /* Invalid packet format */

/* Database errors (-300 to -399) */
#define RTK_ERROR_DATABASE      -300 /* Generic database error */
#define RTK_ERROR_QUERY_FAILED  -301 /* SQL query failed */
#define RTK_ERROR_NO_RESULT     -302 /* Query returned no results */
#define RTK_ERROR_DB_CONNECT    -303 /* Database connection failed */

/* File errors (-400 to -499) */
#define RTK_ERROR_FILE          -400 /* Generic file error */
#define RTK_ERROR_FILE_NOT_FOUND -401 /* File not found */
#define RTK_ERROR_FILE_READ     -402 /* Error reading file */
#define RTK_ERROR_FILE_WRITE    -403 /* Error writing file */
#define RTK_ERROR_FILE_FORMAT   -404 /* Invalid file format */

/* Game logic errors (-500 to -599) */
#define RTK_ERROR_GAME          -500 /* Generic game logic error */
#define RTK_ERROR_INVALID_STATE -501 /* Invalid game state */
#define RTK_ERROR_LEVEL_REQ     -502 /* Level requirement not met */
#define RTK_ERROR_ITEM_REQ      -503 /* Item requirement not met */
#define RTK_ERROR_GOLD_REQ      -504 /* Gold requirement not met */
#define RTK_ERROR_INVENTORY_FULL -505 /* Player inventory full */
#define RTK_ERROR_PERMISSION    -506 /* Permission denied */

/* Player errors (-600 to -699) */
#define RTK_ERROR_PLAYER        -600 /* Generic player error */
#define RTK_ERROR_PLAYER_OFFLINE -601 /* Player is offline */
#define RTK_ERROR_PLAYER_DEAD   -602 /* Player is dead */
#define RTK_ERROR_PLAYER_BUSY   -603 /* Player is busy (in dialog, etc.) */

/*============================================================================
 * Helper Macros - Success/Error Checking
 *============================================================================*/

/**
 * Check if result indicates success
 */
#define RTK_SUCCEEDED(result) ((result) >= 0)

/**
 * Check if result indicates failure
 */
#define RTK_FAILED(result) ((result) < 0)

/**
 * Check if result is exactly RTK_SUCCESS
 */
#define RTK_OK(result) ((result) == RTK_SUCCESS)

/*============================================================================
 * Helper Macros - Validation with Early Return
 *============================================================================*/

/**
 * Return error if pointer is NULL
 * Usage: RTK_CHECK_NULL(pointer, RTK_ERROR_NULL_POINTER);
 */
#define RTK_CHECK_NULL(ptr, error_code) \
    do { \
        if ((ptr) == NULL) { \
            return (error_code); \
        } \
    } while(0)

/**
 * Return error if pointer is NULL, with error message
 * Usage: RTK_CHECK_NULL_MSG(pointer, RTK_ERROR_NULL_POINTER, "player is null");
 */
#define RTK_CHECK_NULL_MSG(ptr, error_code, msg) \
    do { \
        if ((ptr) == NULL) { \
            ShowError("%s: %s\n", __FUNCTION__, (msg)); \
            return (error_code); \
        } \
    } while(0)

/**
 * Return error if condition is false
 * Usage: RTK_CHECK(index < MAX_SIZE, RTK_ERROR_OUT_OF_RANGE);
 */
#define RTK_CHECK(condition, error_code) \
    do { \
        if (!(condition)) { \
            return (error_code); \
        } \
    } while(0)

/**
 * Return error if condition is false, with error message
 * Usage: RTK_CHECK_MSG(index < MAX_SIZE, RTK_ERROR_OUT_OF_RANGE, "index out of bounds");
 */
#define RTK_CHECK_MSG(condition, error_code, msg) \
    do { \
        if (!(condition)) { \
            ShowError("%s: %s\n", __FUNCTION__, (msg)); \
            return (error_code); \
        } \
    } while(0)

/**
 * Return error if value is out of range
 * Usage: RTK_CHECK_RANGE(index, 0, MAX_INDEX, RTK_ERROR_OUT_OF_RANGE);
 */
#define RTK_CHECK_RANGE(value, min, max, error_code) \
    do { \
        if ((value) < (min) || (value) > (max)) { \
            return (error_code); \
        } \
    } while(0)

/*============================================================================
 * Helper Macros - Propagate Errors
 *============================================================================*/

/**
 * Call function and return on error
 * Usage: RTK_TRY(some_function(arg1, arg2));
 */
#define RTK_TRY(expr) \
    do { \
        rtk_result_t _result = (expr); \
        if (RTK_FAILED(_result)) { \
            return _result; \
        } \
    } while(0)

/**
 * Call function and return custom error on failure
 * Usage: RTK_TRY_OR(some_function(arg1), RTK_ERROR_GAME);
 */
#define RTK_TRY_OR(expr, error_code) \
    do { \
        rtk_result_t _result = (expr); \
        if (RTK_FAILED(_result)) { \
            return (error_code); \
        } \
    } while(0)

/*============================================================================
 * Helper Macros - Logging with Return
 *============================================================================*/

/**
 * Log warning and return error
 */
#define RTK_RETURN_WARN(error_code, fmt, ...) \
    do { \
        ShowWarning("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
        return (error_code); \
    } while(0)

/**
 * Log error and return error code
 */
#define RTK_RETURN_ERROR(error_code, fmt, ...) \
    do { \
        ShowError("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
        return (error_code); \
    } while(0)

/**
 * Log debug and return success
 */
#define RTK_RETURN_DEBUG(fmt, ...) \
    do { \
        ShowDebug("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
        return RTK_SUCCESS; \
    } while(0)

/*============================================================================
 * Error Description Function
 *============================================================================*/

/**
 * Get human-readable description of error code
 * @param error_code The error code to describe
 * @return Static string describing the error
 */
static inline const char* rtk_error_string(rtk_result_t error_code)
{
    switch (error_code) {
        /* Success codes */
        case RTK_SUCCESS:           return "Success";
        case RTK_SUCCESS_PARTIAL:   return "Partial success";
        case RTK_SUCCESS_NOOP:      return "Success (no operation)";

        /* General errors */
        case RTK_ERROR:             return "Generic error";
        case RTK_ERROR_INVALID_PARAM: return "Invalid parameter";
        case RTK_ERROR_NULL_POINTER: return "Null pointer";
        case RTK_ERROR_OUT_OF_RANGE: return "Out of range";
        case RTK_ERROR_NOT_FOUND:   return "Not found";
        case RTK_ERROR_ALREADY_EXISTS: return "Already exists";
        case RTK_ERROR_NOT_INIT:    return "Not initialized";
        case RTK_ERROR_TIMEOUT:     return "Timeout";

        /* Memory errors */
        case RTK_ERROR_NO_MEMORY:   return "Out of memory";
        case RTK_ERROR_BUFFER_FULL: return "Buffer full";
        case RTK_ERROR_BUFFER_EMPTY: return "Buffer empty";

        /* Network errors */
        case RTK_ERROR_NETWORK:     return "Network error";
        case RTK_ERROR_DISCONNECT:  return "Disconnected";
        case RTK_ERROR_SEND_FAILED: return "Send failed";
        case RTK_ERROR_RECV_FAILED: return "Receive failed";
        case RTK_ERROR_INVALID_PACKET: return "Invalid packet";

        /* Database errors */
        case RTK_ERROR_DATABASE:    return "Database error";
        case RTK_ERROR_QUERY_FAILED: return "Query failed";
        case RTK_ERROR_NO_RESULT:   return "No result";
        case RTK_ERROR_DB_CONNECT:  return "DB connection failed";

        /* File errors */
        case RTK_ERROR_FILE:        return "File error";
        case RTK_ERROR_FILE_NOT_FOUND: return "File not found";
        case RTK_ERROR_FILE_READ:   return "File read error";
        case RTK_ERROR_FILE_WRITE:  return "File write error";
        case RTK_ERROR_FILE_FORMAT: return "Invalid file format";

        /* Game logic errors */
        case RTK_ERROR_GAME:        return "Game error";
        case RTK_ERROR_INVALID_STATE: return "Invalid state";
        case RTK_ERROR_LEVEL_REQ:   return "Level requirement not met";
        case RTK_ERROR_ITEM_REQ:    return "Item requirement not met";
        case RTK_ERROR_GOLD_REQ:    return "Gold requirement not met";
        case RTK_ERROR_INVENTORY_FULL: return "Inventory full";
        case RTK_ERROR_PERMISSION:  return "Permission denied";

        /* Player errors */
        case RTK_ERROR_PLAYER:      return "Player error";
        case RTK_ERROR_PLAYER_OFFLINE: return "Player offline";
        case RTK_ERROR_PLAYER_DEAD: return "Player dead";
        case RTK_ERROR_PLAYER_BUSY: return "Player busy";

        default:                    return "Unknown error";
    }
}

/*============================================================================
 * Assert Macros (Debug only)
 *============================================================================*/

#ifdef DEBUG
/**
 * Assert condition in debug builds
 */
#define RTK_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            ShowFatalError("%s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #condition); \
        } \
    } while(0)

/**
 * Assert with custom message
 */
#define RTK_ASSERT_MSG(condition, msg) \
    do { \
        if (!(condition)) { \
            ShowFatalError("%s:%d: Assertion failed: %s - %s\n", __FILE__, __LINE__, #condition, (msg)); \
        } \
    } while(0)
#else
#define RTK_ASSERT(condition) ((void)0)
#define RTK_ASSERT_MSG(condition, msg) ((void)0)
#endif

#endif /* _RTK_ERROR_H_ */

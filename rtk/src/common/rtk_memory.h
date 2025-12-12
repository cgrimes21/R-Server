/**
 * rtk_memory.h - Memory management documentation and utilities
 *
 * This header documents the RTK Server memory management conventions
 * and provides additional safety utilities.
 *
 * Part of RTK Server Phase 4.2 documentation.
 *
 * ============================================================================
 * MEMORY MANAGEMENT CONVENTIONS
 * ============================================================================
 *
 * 1. ALLOCATION MACROS (from malloc.h):
 *
 *    CALLOC(result, type, number)
 *    - Allocates zeroed memory for 'number' elements of 'type'
 *    - Stores result in 'result' variable
 *    - EXITS THE PROGRAM if allocation fails (critical failure)
 *    - Always use for structures and arrays
 *
 *    REALLOC(result, type, number)
 *    - Resizes existing allocation to 'number' elements
 *    - May move the memory block
 *
 *    FREE(result)
 *    - Frees memory AND sets pointer to NULL
 *    - Safe to call multiple times (double-free protection)
 *    - ALWAYS use this instead of raw free()
 *
 * 2. NULL POINTER CHECKS (from malloc.h):
 *
 *    nullpo_ret(result, target)
 *    - Returns 'result' if 'target' is NULL
 *    - Use for recoverable null pointer situations
 *
 *    nullpo_chk(target)
 *    - EXITS THE PROGRAM if 'target' is NULL
 *    - Use for critical pointers that should never be NULL
 *
 * 3. STRING DUPLICATION:
 *
 *    aStrdup(string)
 *    - Duplicates a string with tracked allocation
 *    - Returns NULL if input is NULL
 *    - Must be freed with FREE()
 *
 * ============================================================================
 * USAGE GUIDELINES
 * ============================================================================
 *
 * 1. Always use CALLOC for new allocations:
 *    - Zeroes memory (prevents uninitialized data bugs)
 *    - Handles allocation failure automatically
 *    - Tracks allocation for debugging
 *
 *    GOOD:
 *      struct player_data* pd;
 *      CALLOC(pd, struct player_data, 1);
 *
 *    BAD:
 *      struct player_data* pd = malloc(sizeof(struct player_data));
 *
 * 2. Always use FREE to deallocate:
 *    - Sets pointer to NULL (prevents use-after-free)
 *    - Safe for double-free
 *
 *    GOOD:
 *      FREE(pd);
 *      // pd is now NULL
 *
 *    BAD:
 *      free(pd);
 *      // pd is now dangling pointer!
 *
 * 3. Check pointers before use:
 *    - Use nullpo_ret for optional pointers
 *    - Use nullpo_chk for critical pointers
 *    - Use RTK_CHECK_NULL from rtk_error.h for new code
 *
 * 4. Clean up on error paths:
 *    - Free any allocated memory before returning on error
 *    - Use goto cleanup pattern for complex functions
 *
 *    GOOD:
 *      int complex_function() {
 *          struct data* a = NULL;
 *          struct data* b = NULL;
 *
 *          CALLOC(a, struct data, 1);
 *          if (some_error) goto cleanup;
 *
 *          CALLOC(b, struct data, 1);
 *          if (other_error) goto cleanup;
 *
 *          // ... work ...
 *          return 0;
 *
 *      cleanup:
 *          FREE(a);
 *          FREE(b);
 *          return -1;
 *      }
 *
 * 5. Database structures are long-lived:
 *    - item_db, class_db, clan_db, etc. live for server lifetime
 *    - Allocated at startup, freed at shutdown
 *    - Don't free these during normal operation
 *
 * 6. Player/Session data lifecycle:
 *    - Allocated on login/connection
 *    - Freed on logout/disconnect
 *    - Be careful with references after player disconnect
 *
 * ============================================================================
 * MEMORY DEBUGGING
 * ============================================================================
 *
 * When USE_MEMMGR is defined (default for debug builds):
 * - All allocations are tracked with file:line:function
 * - Memory leaks can be detected at shutdown
 * - Double-frees will be logged
 *
 * When NO_MEMMGR is defined:
 * - Uses standard system malloc/free
 * - Faster but no tracking
 * - Use for production builds
 *
 */

#ifndef _RTK_MEMORY_H_
#define _RTK_MEMORY_H_

#include "malloc.h"

/*============================================================================
 * Additional Memory Safety Macros
 *============================================================================*/

/**
 * Safe string copy with null termination
 * Prevents buffer overflow by always null-terminating
 */
#define SAFE_STRCPY(dest, src, size) \
    do { \
        strncpy((dest), (src), (size) - 1); \
        (dest)[(size) - 1] = '\0'; \
    } while(0)

/**
 * Safe string concatenation with null termination
 */
#define SAFE_STRCAT(dest, src, size) \
    do { \
        size_t _len = strlen(dest); \
        if (_len < (size) - 1) { \
            strncat((dest), (src), (size) - _len - 1); \
        } \
    } while(0)

/**
 * Allocate and copy string safely
 * Returns NULL on allocation failure (doesn't exit)
 */
#define SAFE_STRDUP(dest, src) \
    do { \
        if ((src) != NULL) { \
            size_t _len = strlen(src) + 1; \
            (dest) = (char*)aCalloc(_len, sizeof(char)); \
            if ((dest) != NULL) { \
                memcpy((dest), (src), _len); \
            } \
        } else { \
            (dest) = NULL; \
        } \
    } while(0)

/**
 * Free and NULL in one statement (alias for FREE)
 */
#define SAFE_FREE(ptr) FREE(ptr)

/**
 * Check if pointer is valid (not NULL)
 * For use in conditionals
 */
#define IS_VALID_PTR(ptr) ((ptr) != NULL)

/**
 * Zero out a structure
 */
#define ZERO_STRUCT(ptr) memset((ptr), 0, sizeof(*(ptr)))

/**
 * Zero out an array
 */
#define ZERO_ARRAY(arr, count) memset((arr), 0, sizeof((arr)[0]) * (count))

#endif /* _RTK_MEMORY_H_ */

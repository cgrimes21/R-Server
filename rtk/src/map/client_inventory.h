/**
 * @file client_inventory.h
 * @brief Inventory, equipment, and item-related packets
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all inventory/item visual updates.
 */

#ifndef _CLIENT_INVENTORY_H_
#define _CLIENT_INVENTORY_H_

#include <stdarg.h>
#include "map.h"

/* Item add/remove display */
int client_send_add_item(USER* sd, int num);
int client_send_del_item(USER* sd, int num, int type);

/* Equipment */
int client_send_equip(USER* sd, int id);
int client_equip_item(USER* sd, int id);
int client_unequip_item(USER* sd, int spot);

/* Parse item operations */
int client_parse_drop_item(USER* sd);
int client_parse_get_item(USER* sd);
int client_parse_use_item(USER* sd);
int client_parse_eat_item(USER* sd);
int client_parse_unequip(USER* sd);

/* Throw items */
int client_throw_item_script(USER* sd);
int client_throw_item_sub(USER* sd, int id, int type, int x, int y);
int client_throw_check(struct block_list* bl, va_list ap);
int client_throw_confirm(USER* sd);
int client_parse_throw(USER* sd);

/* Inventory checks */
int client_check_inv_bod(USER* sd);

/* Gold/item handling */
int client_drop_gold(USER* sd, unsigned int amounts);
int client_hand_gold(USER* sd);
int client_hand_item(USER* sd);
int client_post_item(USER* sd);

/* Equipment type conversions */
int client_get_slot_from_equip_type(int equipType);
int client_get_equip_type(int val);

#endif /* _CLIENT_INVENTORY_H_ */

/**
 * @file clif_inventory.h
 * @brief Inventory, equipment, and item-related packets
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all inventory/item visual updates.
 */

#ifndef _CLIF_INVENTORY_H_
#define _CLIF_INVENTORY_H_

#include <stdarg.h>
#include "map.h"

/* Item add/remove display */
int clif_sendadditem(USER* sd, int num);
int clif_senddelitem(USER* sd, int num, int type);

/* Equipment */
int clif_sendequip(USER* sd, int id);
int clif_equipit(USER* sd, int id);
int clif_unequipit(USER* sd, int spot);

/* Parse item operations */
int clif_parsedropitem(USER* sd);
int clif_parsegetitem(USER* sd);
int clif_parseuseitem(USER* sd);
int clif_parseeatitem(USER* sd);
int clif_parseunequip(USER* sd);

/* Throw items */
int clif_throwitem_script(USER* sd);
int clif_throwitem_sub(USER* sd, int id, int type, int x, int y);
int clif_throw_check(struct block_list* bl, va_list ap);
int clif_throwconfirm(USER* sd);
int clif_parsethrow(USER* sd);

/* Inventory checks */
int clif_checkinvbod(USER* sd);

/* Gold/item handling */
int clif_dropgold(USER* sd, unsigned int amounts);
int clif_handgold(USER* sd);
int clif_handitem(USER* sd);
int clif_postitem(USER* sd);

/* Equipment type conversions */
int getclifslotfromequiptype(int equipType);
int clif_getequiptype(int val);

#endif /* _CLIF_INVENTORY_H_ */

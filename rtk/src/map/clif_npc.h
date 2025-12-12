/**
 * @file clif_npc.h
 * @brief NPC dialogs, menus, shops, and interaction packets
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all NPC interaction UI.
 */

#ifndef _CLIF_NPC_H_
#define _CLIF_NPC_H_

#include <stdarg.h>
#include "map.h"

/* Script messages/dialogs */
int clif_scriptmes(USER* sd, int id, char* msg, int previous, int next);
int clif_scriptmenu(USER* sd, int id, char* dialog, char* menu[], int size);
int clif_scriptmenuseq(USER* sd, int id, char* dialog, char* menu[], int size, int previous, int next);
int clif_parsemenu(USER* sd);
int clif_parsenpcdialog(USER* sd);

/* Buy/sell dialogs */
int clif_buydialog(USER* sd, unsigned int id, char* dialog, struct item* item, int price[], int count);
int clif_parsebuy(USER* sd);
int clif_selldialog(USER* sd, unsigned int id, char* dialog, int item[], int count);
int clif_parsesell(USER* sd);

/* Input dialogs */
int clif_input(USER* sd, int id, char* dialog, char* item);
int clif_inputseq(USER* sd, int id, char* dialog, char* dialog2, char* dialog3, char* menu[], int size, int previous, int next);
int clif_parseinput(USER* sd);

/* Look at/click functions */
int clif_parselookat(USER* sd);
int clif_parselookat_2(USER* sd);
int clif_parselookat_sub(struct block_list* bl, va_list ap);
int clif_parselookat_scriptsub(USER* sd, struct block_list* bl);
int clif_clickonplayer(USER* sd, struct block_list* bl);

/* Hair/face menu */
int clif_hairfacemenu(USER* sd, char* dialog, char* menu[], int size);

/* Map message numbers */
int clif_mapmsgnum(USER* sd, int id);

#endif /* _CLIF_NPC_H_ */

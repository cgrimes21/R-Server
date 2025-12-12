
%.o: %.c
	@echo -e " Compile... $<"
	@$(CC) $(CFLAGS) -c -o  $@ $<

all: map-server

# Note: clif_*.c modules contain extracted functions but clif.c still has originals
# To complete extraction: remove duplicate functions from clif.c
# For now, compile only clif.c (modules are header-only wrappers)
CLIF_OBJ = clif.o
# Note: sl_blocklist.c contains extracted bll_* functions but sl.c still has originals
# Note: sl_player.c contains extracted pcl_* functions but sl.c still has originals
# Note: sl_registry.c contains extracted registry types but sl.c still has originals
# To complete extraction: remove functions from sl.c, then add sl_blocklist.o sl_player.o sl_registry.o
SL_OBJ = sl.o sl_types.o

map-server: map.o $(CLIF_OBJ) $(SL_OBJ) intif.o class_db.o itemdb.o recipedb.o pc.o npc.o script.o command.o mob.o magic.o board_db.o clan_db.o creation.o guide.o $(COMMON_OBJ)
	@echo -e " Linking... $<"
	@$(CC) -o ../../$@ map.o $(CLIF_OBJ) $(SL_OBJ) intif.o class_db.o itemdb.o recipedb.o pc.o npc.o script.o command.o mob.o magic.o board_db.o clan_db.o creation.o guide.o $(COMMON_OBJ) $(CLIBS)

map.o: map.c map.h $(COMMON_H)
clif.o: clif.c clif.h $(COMMON_H)
clif_crypto.o: clif_crypto.c clif_crypto.h $(COMMON_H)
clif_chat.o: clif_chat.c clif_chat.h $(COMMON_H)
clif_visual.o: clif_visual.c clif_visual.h $(COMMON_H)
clif_combat.o: clif_combat.c clif_combat.h $(COMMON_H)
clif_inventory.o: clif_inventory.c clif_inventory.h $(COMMON_H)
clif_npc.o: clif_npc.c clif_npc.h $(COMMON_H)
clif_player.o: clif_player.c clif_player.h $(COMMON_H)
intif.o: intif.c intif.h $(COMMON_H)
class_db.o: class_db.c class_db.h $(COMMON_H)
itemdb.o: itemdb.c itemdb.h $(COMMON_H)
recipedb.o: recipedb.c recipedb.h $(COMMON_H)
pc.o: pc.c pc.h $(COMMON_H)
npc.o: npc.c npc.h $(COMMON_H)
script.o: script.c script.h $(COMMON_H)
command.o: command.c command.h $(COMMON_H)
mob.o: mob.c mob.h $(COMMON_H)
magic.o: magic.c magic.h $(COMMON_H)
board_db.o: board_db.c board_db.h $(COMMON_H)
clan_db.o: clan_db.c clan_db.h $(COMMON_H)
sl.o: sl.c sl.h sl_types.h sl_blocklist.h $(COMMON_H)
sl_types.o: sl_types.c sl_types.h sl.h $(COMMON_H)
sl_blocklist.o: sl_blocklist.c sl_blocklist.h sl_types.h sl.h $(COMMON_H)
sl_player.o: sl_player.c sl_player.h sl_types.h sl.h $(COMMON_H)
sl_registry.o: sl_registry.c sl_registry.h sl_types.h sl.h $(COMMON_H)
sl_item.o: sl_item.c sl_item.h sl_types.h sl.h $(COMMON_H)
sl_npc.o: sl_npc.c sl_npc.h sl_types.h sl.h $(COMMON_H)
sl_mob.o: sl_mob.c sl_mob.h sl_types.h sl.h $(COMMON_H)
creation.o: creation.c creation.h $(COMMON_H)
guide.o: guide.c guide.h $(COMMON_H)
clean:
	@rm -f *.o ../../map-server

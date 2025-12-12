
%.o: %.c
	@echo -e " Compile... $<"
	@$(CC) $(CFLAGS) -c -o  $@ $<

all: map-server

# Client layer - monolithic client.c with split modules
CLIENT_OBJ = client.o client_crypto.o client_chat.o client_visual.o client_combat.o client_inventory.o client_npc.o client_player.o

# Lua binding layer - fully split into modules
LUA_OBJ = lua_core.o lua_types.o lua_blocklist.o lua_player.o lua_mob.o lua_npc.o lua_item.o lua_registry.o

map-server: map.o $(CLIENT_OBJ) $(LUA_OBJ) intif.o class_db.o itemdb.o recipedb.o pc.o npc.o script.o command.o mob.o magic.o board_db.o clan_db.o creation.o guide.o $(COMMON_OBJ)
	@echo -e " Linking... $<"
	@$(CC) -o ../../$@ map.o $(CLIENT_OBJ) $(LUA_OBJ) intif.o class_db.o itemdb.o recipedb.o pc.o npc.o script.o command.o mob.o magic.o board_db.o clan_db.o creation.o guide.o $(COMMON_OBJ) $(CLIBS)

map.o: map.c map.h $(COMMON_H)
client.o: client.c client.h $(COMMON_H)
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
lua_core.o: lua_core.c lua_core.h lua_types.h lua_blocklist.h $(COMMON_H)
lua_types.o: lua_types.c lua_types.h $(COMMON_H)
lua_blocklist.o: lua_blocklist.c lua_blocklist.h lua_core.h $(COMMON_H)
lua_player.o: lua_player.c lua_player.h lua_core.h $(COMMON_H)
lua_mob.o: lua_mob.c lua_mob.h lua_core.h $(COMMON_H)
lua_npc.o: lua_npc.c lua_npc.h lua_core.h $(COMMON_H)
lua_item.o: lua_item.c lua_item.h lua_core.h $(COMMON_H)
lua_registry.o: lua_registry.c lua_registry.h lua_core.h $(COMMON_H)
client_crypto.o: client_crypto.c client_crypto.h client.h $(COMMON_H)
client_chat.o: client_chat.c client_chat.h client.h $(COMMON_H)
client_visual.o: client_visual.c client_visual.h client.h $(COMMON_H)
client_combat.o: client_combat.c client_combat.h client.h $(COMMON_H)
client_inventory.o: client_inventory.c client_inventory.h client.h $(COMMON_H)
client_npc.o: client_npc.c client_npc.h client.h $(COMMON_H)
client_player.o: client_player.c client_player.h client.h $(COMMON_H)
creation.o: creation.c creation.h $(COMMON_H)
guide.o: guide.c guide.h $(COMMON_H)
clean:
	@rm -f *.o ../../map-server

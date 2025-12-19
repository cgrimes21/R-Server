# RTK Packet Opcodes

Opcode byte at position 3: WFIFOB(fd, 3) / RFIFOB(fd, 3)

## Client to Server (C2S)

```
0x05  client_parsemap             Map request
0x06  client_parsewalk            Walk/movement
0x07  client_parsegetitem         Pick up item
0x08  client_parsedropitem        Drop item
0x09  client_parselookat_2        Look at (type 2)
0x0A  client_parselookat          Look at object
0x0B  client_close_it             Close/disconnect
0x0C  client_handle_missing_object  Missing object request
0x0D  client_parseignore          Ignore player
0x0E  client_parsesay             Chat message
0x0F  client_parsemagic           Cast spell
0x10  client_accept2              Login/accept (pre-session)
0x11  client_parseside            Change facing direction
0x12  client_parsewield           Wield/equip item
0x13  client_parseattack          Attack
0x17  client_parsethrow           Throw item
0x18  client_user_list            Request user list
0x19  client_parsewisp            Whisper message
0x1A  client_parseeatitem         Eat/consume item
0x1B  client_change_status        Change status
0x1C  client_parseuseitem         Use item
0x1D  client_parseemotion         Emotion/emote
0x1E  client_parsewield           Wield (alternate)
0x1F  client_parseunequip         Unequip item
0x20  client_open_sub             Open/interact (O key)
0x23  client_paper_popup_write_save  Save paper popup
0x24  client_drop_gold            Drop gold
0x27  -                           Quest tab / Shift+Z
0x29  client_hand_item            Hand item to player
0x2A  client_hand_gold            Hand gold to player
0x2D  client_my_status            Status request
0x2E  client_add_group            Add to group
0x30  client_parsechangespell     Change spell/position
0x32  client_parsewalk            Walk (alternate)
0x34  client_post_item            Post item
0x38  client_refresh              Refresh
0x39  client_handle_menu_input    Menu/input response
0x3A  client_parsenpcdialog       NPC dialog
0x3B  client_handle_boards        Boards
0x3F  pc_warp                     Map change/warp
0x41  client_parseparcel          Parcel
0x42  -                           Client crash debug
0x43  client_handle_click_get_info  Click get info
0x4A  client_parse_exchange       Exchange/trade
0x4C  client_handle_powerboards   Power boards
0x4F  client_change_profile       Profile change
0x60  -                           Ping
0x66  client_send_towns           Towns request
0x69  -                           Obstruction report
0x6B  createdb_start              Creation system
0x73  client_send_board           Web board/profile
0x75  client_parsewalkpong        Walk pong
0x77  client_parsefriends         Friends list
0x7B  send_meta                   Item info request
0x7C  client_send_minimap         Minimap request
0x7D  client_parseranking         Ranking system
0x82  client_parseviewchange      View change
0x83  -                           Screenshots
0x84  client_hunter_toggle        Hunter list toggle
0x85  client_send_hunter_note     Hunter note
```

## Server to Client (S2C)

```
0x03  client_send_status          Send status
0x04  client_send_xy              Send coordinates
0x05  client_sendtime             Send time
0x07  client_sendid               Send player ID
0x08  client_send_update_status   Update status
0x0A  client_sendsay              Chat message
0x0E  client_lookgone             Object disappeared
0x0F  client_senddelitem          Delete item
0x10  client_sendadditem          Add item
0x12  client_sendmapinfo          Map info
0x13  client_send_selfbar         Health bar (self)
0x15  client_sendack              Acknowledge
0x17  client_sendmagic            Magic/spell
0x18  client_user_list            User list
0x19  client_sendwisp             Whisper
0x1B  client_sendbluemessage      Blue message
0x1E  client_sendxy               Send XY
0x20  client_sendxychange         XY change
0x22  client_refresh              Refresh
0x26  client_grouphealth_update   Group health
0x29  client_sendanimation        Animation
0x2E  client_showboards           Show boards
0x2F  client_selldialog           Shop dialog
0x30  client_scriptmes            Script message/menu
0x31  client_send_minitext        Mini text
0x33  client_charlook_sub         Character look
0x35  client_sendscriptsay        Script say
0x36  intif_sendmail              Mail
0x38  client_sendequip            Equipment
0x3B  client_spawn                Spawn
0x3D  client_sendboard            Board
0x42  client_exchange_*           Exchange/trade
0x46  client_sendpowerboard       Power board
0x49  client_sendmapdata          Map data
0x4E  client_throwitem_sub        Throw item
0x58  client_popup                Popup
0x59  client_sendoptions          Options
0x5A  client_sendxynoclick        XY no click
0x5F  client_lookgone             Look gone (mob)
0x62  client_send_profile         Profile/board URL
0x63  client_leave_group          Leave group
0x66  client_send_url             Send URL
0x67  client_sendtowns            Towns list
0x68  client_sendmapinfo          Map info (extended)
0x6F  client_send_aether          Aether
0x70  client_send_minimap         Minimap
0x7D  client_parseranking         Ranking
0x83  client_send_reward_info     Reward info
0x84  client_get_reward           Get reward
```

---

## Login Server (login_client.c)

### Client to Login Server (C2S)

```
0x00  -                           Version check / handshake
0x02  -                           Character creation request
0x03  -                           Login request
0x04  -                           Character creation data (face/hair/etc)
0x10  -                           Connection init
0x26  -                           Password change
0x57  -                           Multi-server (unused)
0x62  -                           Unknown
0x71  -                           Ping
0x7B  send_meta/send_metalist     Request item metadata
0xFF  intif_auth                  Auth request
```

### Login Server to Client (S2C)

```
0x00  -                           Version response / patch info
0x02  login_client_message        Error/status message
0x60  -                           Connection ack
0x66  login_client_sendurl        Send URL
0x6F  send_metafile/send_metalist Metadata file/list
0x7E  -                           Connected banner
```

---

## Inter-Server Communication

### Login <-> Char Server (0x1xxx / 0x2xxx)

```
Login -> Char (0x1xxx):
0x1000  logif_parse_accept        Connection accept
0x1001  logif_parse_usedname      Check name availability
0x1002  logif_parse_newchar       New character creation
0x1003  logif_parse_login         Login request
0x1004  logif_parse_setpass       Password change

Char -> Login (0x2xxx):
0x2001  intif_parse_2001          Response
0x2002  intif_parse_2002          Response
0x2003  intif_parse_connectconfirm  Connection confirm / auth result
0x2004  intif_parse_changepass    Password change result
```

### Map <-> Char Server (0x3xxx / 0x38xx)

```
Map -> Char (0x3xxx):
0x3000  -                         Connection init
0x3001  mapif_parse_mapset        Map server registration
0x3002  mapif_parse_login         Player login
0x3003  mapif_parse_requestchar   Request character data
0x3004  mapif_parse_savechar      Save character
0x3005  mapif_parse_logout        Player logout
0x3007  mapif_parse_savecharlog   Save character log
0x3008  mapif_parse_deletepost    Delete board post
0x3009  mapif_parse_showposts     Show board posts
0x300A  mapif_parse_readpost      Read board post
0x300B  mapif_parse_userlist      User list request
0x300C  mapif_parse_boardpost     Board post
0x300D  mapif_parse_nmailwrite    Write mail
0x300E  mapif_parse_findnewmp     Find new MP
0x300F  mapif_parse_nmailwritecopy  Mail write copy

Char -> Map (0x38xx):
0x3800  intif_parse_accept        Connection accept
0x3801  intif_parse_mapset        Map set response
0x3802  intif_parse_authadd       Auth add
0x3803  intif_parse_charload      Character data
0x3804  intif_parse_checkonline   Online check result
0x3805  intif_parse_unknown       Unknown
0x3808  intif_parse_deletepostresponse  Delete post result
0x3809  intif_parse_showpostresponse    Show posts result
0x380A  intif_parse_userlist      User list response
0x380B  intif_parse_boardpostresponse   Board post result
0x380C  intif_parse_nmailwriteresponse  Mail write result
0x380D  intif_parse_findmp        Find MP result
0x380E  intif_parse_setmp         Set MP
0x380F  intif_parse_readpost      Read post response
```

### Char <-> Save Server (0x4xxx)

```
0x4000  saveif_parse_accept       Save server connection
```

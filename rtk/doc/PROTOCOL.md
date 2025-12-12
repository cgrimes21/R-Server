# RTK Server Packet Protocol Documentation

This document describes the packet protocol used for client-server communication in RTK Server.

## Packet Structure

All packets follow this structure:

```
+--------+--------+--------+--------+--------+--------+----...----+--------+--------+--------+
| Marker | Length (BE)     | Opcode | PktInc |     Data...        | K2-Lo  |  K1    | K2-Hi  |
+--------+--------+--------+--------+--------+--------+----...----+--------+--------+--------+
   0xAA     [1]      [2]      [3]      [4]       [5+]      [-3]      [-2]      [-1]
```

### Header (Bytes 0-2)
| Offset | Size | Name | Description |
|--------|------|------|-------------|
| 0 | 1 | Marker | Always `0xAA` - packet start marker |
| 1-2 | 2 | Length | Packet length in big-endian (includes header, excludes trailer) |

### Body (Bytes 3+)
| Offset | Size | Name | Description |
|--------|------|------|-------------|
| 3 | 1 | Opcode | Packet type identifier |
| 4 | 1 | PktInc | Packet increment (used in encryption) |
| 5+ | var | Data | Packet-specific payload |

### Trailer (Last 3 bytes, encrypted packets only)
| Offset | Size | Name | Description |
|--------|------|------|-------------|
| -3 | 1 | K2-Lo | Key2 low byte |
| -2 | 1 | K1 | Key1 value |
| -1 | 1 | K2-Hi | Key2 high byte |

## Byte Order

- **Length field**: Big-endian (network byte order)
- **Multi-byte data**: Generally big-endian, use `SWAP16()` / `SWAP32()`
- **Trailer K2**: Low byte first, high byte last (little-endian split)

## Encryption

### Two Encryption Methods

1. **Key1 Encryption** (`rtk_crypt`)
   - Uses static encryption key: `"Urk#nI7ni"` (9 bytes)
   - Simpler, used for specific packet types

2. **Key2 Encryption** (`rtk_crypt2`)
   - Uses dynamic key derived from player's `EncHash`
   - Key generated using MD5-based hash table
   - More secure, used for most packets

### Packet Type to Key Mapping

**Client→Server (Key1 packets)**:
```
2, 3, 4, 11, 21, 38, 58, 66, 67, 75, 80, 87, 98, 113, 115, 123
```
All other client packets use Key2.

**Server→Client (Key1 packets)**:
```
2, 3, 10, 64, 68, 94, 96, 98, 102, 111
```
All other server packets use Key2.

### Encryption Algorithm

```c
// Three-stage XOR encryption
for (i = 0; i < packet_len; i++) {
    // Stage 1: XOR with key (static or dynamic)
    data[i] ^= key[i % 9];

    // Stage 2: XOR with group counter (skip if matches pkt_inc)
    KeyVal = (Group % 256);
    if (KeyVal != packet_inc)
        data[i] ^= KeyVal;

    // Stage 3: XOR with packet increment
    data[i] ^= packet_inc;

    // Advance group every 9 bytes
    GroupCount++;
    if (GroupCount == 9) {
        Group++;
        GroupCount = 0;
    }
}
```

### Key2 Generation

For Key2 encryption, the dynamic key is generated:

1. Server generates random k1, k2 values in `set_packet_indexes()`
2. k1 and k2 are XORed with magic values (0x21/0x7424 for server, 0x25/0x2361 for client)
3. 9-byte key is extracted from player's 1024-byte hash table:
   ```c
   for (i = 0; i < 9; i++) {
       keyout[i] = table[(k1 * i + k2) & 0x3FF];
       k1 += 3;
   }
   ```

## Common Packet Types

### Server→Client Opcodes

| Opcode | Name | Description |
|--------|------|-------------|
| 0x02 | Login Response | Authentication result |
| 0x03 | Character Data | Character information |
| 0x04 | Spawn Player | Player spawn in area |
| 0x07 | Move | Movement packet |
| 0x08 | Animation | Play animation |
| 0x0A | Chat | Chat message |
| 0x0B | Whisper | Private message |
| 0x10 | Item Add | Add item to inventory |
| 0x11 | Item Remove | Remove from inventory |
| 0x19 | Status Update | Stat/status change |
| 0x1B | Paper Popup | Writable paper dialog |
| 0x33 | NPC Dialog | NPC text dialog |
| 0x35 | Paper Display | Read-only paper |
| 0x36 | Menu | NPC menu options |
| 0x62 | Board/URL | Board or URL packet |
| 0x66 | Browser | Open browser |

### Client→Server Opcodes

| Opcode | Name | Description |
|--------|------|-------------|
| 0x02 | Login | Login request |
| 0x03 | Walk | Movement request |
| 0x04 | Attack | Attack action |
| 0x06 | Use Item | Use item from inventory |
| 0x07 | Drop Item | Drop item |
| 0x08 | Pick Up | Pick up ground item |
| 0x0A | Chat | Send chat message |
| 0x0B | Whisper | Send whisper |
| 0x0F | Menu Select | NPC menu selection |
| 0x11 | NPC Click | Click on NPC |
| 0x13 | Equip | Equip item |
| 0x14 | Unequip | Unequip item |

## Buffer Macros

### Write Macros (Send)
```c
WFIFOB(fd, offset) // Write byte at offset
WFIFOW(fd, offset) // Write word (2 bytes) at offset
WFIFOL(fd, offset) // Write long (4 bytes) at offset
WFIFOP(fd, offset) // Get pointer at offset
WFIFOSET(fd, len)  // Send packet of length len
```

### Read Macros (Receive)
```c
RFIFOB(fd, offset) // Read byte at offset
RFIFOW(fd, offset) // Read word (2 bytes) at offset
RFIFOL(fd, offset) // Read long (4 bytes) at offset
RFIFOP(fd, offset) // Get pointer at offset
RFIFOSKIP(fd, len) // Skip len bytes in buffer
```

## Packet Construction Example

```c
void send_chat_message(USER* sd, const char* message) {
    int len = strlen(message);
    int fd = sd->fd;

    WFIFOB(fd, 0) = 0xAA;                    // Marker
    WFIFOW(fd, 1) = SWAP16(len + 5);         // Length (big-endian)
    WFIFOB(fd, 3) = 0x0A;                    // Opcode: Chat
    WFIFOB(fd, 4) = 0x03;                    // Sub-type
    WFIFOB(fd, 5) = 0x08;                    // Chat type
    WFIFOW(fd, 6) = SWAP16(len);             // Message length
    memcpy(WFIFOP(fd, 8), message, len);     // Message data

    WFIFOSET(fd, encrypt(fd));               // Encrypt and send
}
```

## Packet Parsing Example

```c
int parse_walk(int fd, USER* sd) {
    // After decryption, packet is at RFIFOP(fd, 0)
    int x = RFIFOW(fd, 5);  // X coordinate
    int y = RFIFOW(fd, 7);  // Y coordinate

    // Process movement...

    RFIFOSKIP(fd, packet_len);  // Advance read pointer
    return 0;
}
```

## Files Reference

| File | Purpose |
|------|---------|
| `client.c` | Main packet handling |
| `client_crypto.c` | Encryption/decryption |
| `crypt.c` | Encryption algorithms |
| `socket.c` | Network I/O, WFIFO/RFIFO |
| `conf/crypto.conf` | Configurable key mappings |

## Configuration

Packet-to-key mappings can be configured in `conf/crypto.conf`:
```conf
client_key2: 6, 8, 9, 10, 15, 19, 23, 26, 28, 41, 45, 46, 50, 57
server_key2: 4, 7, 8, 11, 12, 19, 23, 24, 51, 54, 57, 64, 99
server_key1_packets: 2, 3, 10, 64, 68, 94, 96, 98, 102, 111
client_key1_packets: 2, 3, 4, 11, 21, 38, 58, 66, 67, 75, 80, 87, 98, 113, 115, 123
```

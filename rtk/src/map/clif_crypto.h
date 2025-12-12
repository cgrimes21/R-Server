/**
 * @file clif_crypto.h
 * @brief Packet encryption/decryption for client communication
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all packet encryption and decryption operations.
 */

#ifndef _CLIF_CRYPTO_H_
#define _CLIF_CRYPTO_H_

#include "map.h"

/* Key arrays for packet encryption */
extern const unsigned char clkey2[];
extern const unsigned char svkey2[];
extern const unsigned char svkey1packets[];
extern const unsigned char clkey1packets[];

/**
 * Check if packet type uses key1 encryption (server -> client)
 * @param packetType The packet type byte
 * @return 1 if uses key2, 0 if uses key1
 */
int isKey(int packetType);

/**
 * Check if packet type uses key1 encryption (client -> server)
 * @param packetType The packet type byte
 * @return 1 if uses key2, 0 if uses key1
 */
int isKey2(int packetType);

/**
 * Encrypt an outgoing packet
 * @param fd File descriptor of the client connection
 * @return Length of the encrypted packet
 */
int encrypt(int fd);

/**
 * Decrypt an incoming packet
 * @param fd File descriptor of the client connection
 * @return 0 on success
 */
int decrypt(int fd);

#endif /* _CLIF_CRYPTO_H_ */

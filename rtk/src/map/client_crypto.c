/**
 * @file client_crypto.c
 * @brief Packet encryption/decryption for client communication
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all packet encryption and decryption operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_crypto.h"
#include "socket.h"
#include "crypt.h"
#include "map.h"

/* Encryption key arrays */
const unsigned char clkey2[] = { 6,8,9,10,15,19,23,26,28,41,45,46,50,57 };
const unsigned char svkey2[] = { 4,7,8,11,12,19,23,24,51,54,57,64,99 };
const unsigned char svkey1packets[] = { 2,3,10,64,68,94,96,98,102,111 };
const unsigned char clkey1packets[] = { 2,3,4,11,21,38,58,66,67,75,80,87,98,113,115,123 };

int client_is_key2(int fd)
{
	int x = 0;
	for (x = 0; x < (sizeof(clkey1packets) / sizeof(clkey1packets[0])); x++)
	{
		if (fd == clkey1packets[x])
			return 0;
	}

	return 1;
}

int client_is_key(int fd)
{
	int x = 0;
	for (x = 0; x < (sizeof(svkey1packets) / sizeof(svkey1packets[0])); x++)
	{
		if (fd == svkey1packets[x])
			return 0;
	}
	return 1;
}

int client_encrypt(int fd)
{
	USER* sd = NULL;
	char key[16];
	sd = (USER*)session[fd]->session_data;
	nullpo_ret(0, sd);
	set_packet_indexes(WFIFOP(fd, 0));

	if (client_is_key(WFIFOB(fd, 3)))
	{
		generate_key2(WFIFOP(fd, 0), &(sd->EncHash), &(key), 0);
		rtk_crypt2(WFIFOP(fd, 0), &(key));
	}
	else {
		rtk_crypt(WFIFOP(fd, 0));
	}
	return (int)SWAP16(*(unsigned short*)WFIFOP(fd, 1)) + 3;
}

int client_decrypt(int fd)
{
	USER* sd = NULL;
	char key[16];
	sd = (USER*)session[fd]->session_data;
	nullpo_ret(0, sd);

	if (client_is_key2(RFIFOB(fd, 3)))
	{
		generate_key2(RFIFOP(fd, 0), &(sd->EncHash), &(key), 1);
		rtk_crypt2(RFIFOP(fd, 0), &(key));
	}
	else {
		rtk_crypt(RFIFOP(fd, 0));
	}
	return 0;
}

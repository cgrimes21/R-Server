/**
 * @file login_client.h
 * @brief Login server client interface
 *
 * Handles client communication for the login server.
 * Renamed from clif.h as part of the RTK naming refactor.
 */

#ifndef _LOGIN_CLIENT_H_
#define _LOGIN_CLIENT_H_

struct login_session_data {
	unsigned int id;
	char name[16];
	char pass[16];
	char face, country, totem, sex, hair, face_color, hair_color;
};

int login_client_message(int, char, char*);
int login_client_sendurl(int, int, char*);

int login_client_accept(int);
int login_client_parse(int);

#endif /* _LOGIN_CLIENT_H_ */

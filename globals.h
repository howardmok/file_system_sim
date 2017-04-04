#ifdef DEBUG
#define _(x) x
#else
#define _(x)
#endif


#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <algorithm>
#include "fs_server.h"	
#include <iostream>
#include <cassert>
#include <cctype>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <queue>
#include "rw_lock.h"
#include <mutex>

// sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * Globals to keep track of data
 */
extern std::unordered_map<std::string, std::string> user_list;
extern std::unordered_map<std::string,std::unordered_map<unsigned int, unsigned int>> sessionInfo; // unordered_map of <username, unordered_map of <sessionId, sequence> >
extern std::unordered_map<unsigned int, rw_lock> blockLock; 
extern std::string FS_MAX_ENCRYPTED_TEXT;
extern unsigned int sessionID;

/*
 * Constants
 */
const size_t MSGSIZE = 1024 + 1;

/*
 * Helper Data Types
 */
struct populate_data {
	uint32_t diskBlock;
	char blockType;
};

/*
 * Functions to help the server function
 */

// Read <username> <password> pairs from stdin
bool isBadPath(std::string);
void read_userlist();

unsigned int find_request_size_start_loc(const char *, unsigned int);

// Extract encrypted text size from cleartext
std::string extract_encrypted_text_size(const char *, unsigned int, unsigned int);

bool populate();

unsigned int traverse(std::string, std::string, unsigned int, std::string, bool);

unsigned int start_session(std::string, unsigned int);

bool create_inode(std::string, std::string, char);

bool delete_inode(std::string, std::string);

std::string read_file(std::string, std::string, unsigned int);

bool write_file(std::string, std::string, unsigned int, std::string);

// returns a string with the response message
std::string parse_message(std::string, std::string, unsigned int);

// returns a nonzero integer free block
unsigned int get_free_blocks();

// sends the server response to the client
bool send_server_response(int, const char *, const unsigned int);

_(
	// Cout functions for debugging purposes
	void err_cout(std::string err_msg);
	void err_cout(const char * str, unsigned int len);
)

#endif // GLOBALS_H_
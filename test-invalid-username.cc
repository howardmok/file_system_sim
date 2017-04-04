/*
 * invalid username test (fs_session)
 * a user should be able to create multple sessions
 * a session requires a valid username and password pair
 * sequences are strictly increasing and do not need to increment by 1
 */

#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session1, session2, seq=0;

    if (argc != 3) {
        // Error checking to make sure user passes in 2 additional arguments
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }

    server = argv[1]; // name of the server
    server_port = atoi(argv[2]); // port number server is filling out requests

    // init client with name of server and port number
    fs_clientinit(server, server_port); 

    // ERR: correct username but incorrect password
    fs_session("user1", "password12", &session1, seq++);

    // ERR: incorrect username but correct password
    fs_session("user12","password1", &session1,seq++);

    // ERR: correct username but mismatched password
    fs_session("user2", "password1", &session1, seq++);

    // correct username and correct password! 
    fs_session("user3", "password3", &session1, seq++);

    // ERR: correct username but mismatched password
    fs_session("user3", "password1", &session2, 0);

    fs_session("user3", "password3", &session2, 1);

    // increment sequence by an arbitrary amount
    seq = seq+5;

    // create another session for user3
    fs_session("user3", "password3", &session1, seq++);
}
/*
 * tests edge cases for duplicate checking for files/directories
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session, seq=0;

    if (argc != 3) {
        // Error checking to make sure user passes in 2 additional arguments
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }

    server = argv[1]; // name of the server
    server_port = atoi(argv[2]); // port number server is filling out requests

    // init client with name of server and port number
    fs_clientinit(server, server_port); 

    // create a user session. note that session is uninitialized and will receive a value from the server.
    // pass in seq=0 and then increment seq by one
    fs_session("", "password1", &session, seq++);

    fs_session("  ", "password1", &session, seq++);

    fs_session("user1", "password1", &session, seq++);

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/usemeupinside", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/usemeupinside/saveme!", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/usemeupinside//saveme!", 'd');
}
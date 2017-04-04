/*
 * tests create and session for input errors in session number and creating bad files.
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session1, session2, seq1=0, seq2 = 0;

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
    // pass in seq1=0 and then increment seq by one
    fs_session("user1", "password1", &session1, seq1++);

    // create a user session. note that session is uninitialized and will receive a value from the server.
    // should fail. wrong password.
    fs_session("user2", "password1", &session2, seq2++);

    // create a user session. note that session is uninitialized and will receive a value from the server.
    /// pass in seq2=0 and then increment seq2 by one
    fs_session("user2", "password2", &session2, seq2++);

    // creates a new file in the directory /me.txt
    // because bad sequence number
    fs_create("user2", "password2", session2, 0, "/me.txt", 'f');

    // creates a new file in the directory /me.txt
    // because bad sequence number. tests whether or not it incorrectly assigns sequence number
    fs_create("user2", "password2", session2, 1, "/me.txt", 'f');

    // creates a new file in the directory /me.txt
    // should successfully create
    fs_create("user2", "password2", session2, seq2++, "/me.txt", 'f');

    // creates a new file in the directory /me.txt.
    // should fail to create. wrong session number
    fs_create("user2", "password2", session1, seq2++, "/you.txt", 'f');

    // creates a new file in the directory /me.txt.
    // should fail to create. wrong session number
    fs_create("user1", "password1", session2, seq1++, "/you", 'd');

    // creates a new directory under /me.txt
    // should fail to create
    fs_create("user2", "password2", session2, seq2++, "/me.txt/wat", 'd');

    // creates a new file under /me.txt
    // should fail to create
    fs_create("user2", "password2", session2, seq2++, "/me.txt/wat", 'f');
}
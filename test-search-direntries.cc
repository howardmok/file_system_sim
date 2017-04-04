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
    fs_session("user1", "password1", &session, seq++);

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode0", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode1", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode2", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode3", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode4", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode5", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode6", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode7", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode8", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode9", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode10", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode11", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode12", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode13", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/inode14", 'd');

    // creates a new file in the directory /
    fs_create("user1", "password1", session, seq++, "/target", 'd');

    // start deleting a bunch of directories from the first "/"" data block
    fs_delete("user1", "password1", session, seq++, "/inode2");
    fs_delete("user1", "password1", session, seq++, "/inode4");
    fs_delete("user1", "password1", session, seq++, "/inode5");
    fs_delete("user1", "password1", session, seq++, "/inode6");

    // start deleting a bunch of things from the second "/" data block
    fs_delete("user1", "password1", session, seq++, "/inode8");
    fs_delete("user1", "password1", session, seq++, "/inode9");
    fs_delete("user1", "password1", session, seq++, "/inode10");
    fs_delete("user1", "password1", session, seq++, "/inode11");
    fs_delete("user1", "password1", session, seq++, "/inode12");
    fs_delete("user1", "password1", session, seq++, "/inode13");
    fs_delete("user1", "password1", session, seq++, "/inode14");

    // creates a new file in the directory /
    // should fail
    fs_create("user1", "password1", session, seq++, "/target", 'd');
}
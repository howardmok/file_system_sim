/*
 * tests trying to delete files that arent yours and create files in directories that arent yours
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session3, session4, seq3=0, seq4 = 0;

    if (argc != 3) {
        // Error checking to make sure user passes in 2 additional arguments
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    
    const char *writedata = "LOL We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";


    server = argv[1]; // name of the server
    server_port = atoi(argv[2]); // port number server is filling out requests

    // init client with name of server and port number
    fs_clientinit(server, server_port);

    // creates a session for user3
    fs_session("user3", "password3", &session3, seq3++);

    // creates a session for user4
    fs_session("user4", "password4", &session4, seq4++);

    // create a directoy that is owned by user3
    fs_create("user3", "password3", session3, seq3++, "/user3", 'd');

    // create a directoy that is owned by user4
    fs_create("user4", "password4", session4, seq4++, "/user4", 'd');

    // create a file that is owned by user3
    fs_create("user3", "password3", session3, seq3++, "/user3file", 'f');

    // create a file that is owned by user4
    fs_create("user4", "password4", session4, seq4++, "/user4file", 'f');

    // can't write to file you don't own
    fs_writeblock("user3", "password3", session3, seq3++, "/user4file", 0, writedata);

    // can't write to file you don't own
    fs_writeblock("user4", "password4", session4, seq4++, "/user3file", 0, writedata);

    // delete a file that is not owned by you
    // should fail
    fs_delete("user3", "password3", session3, seq3++, "/user4");

    // create a subdirectory that is owned by user3
    fs_create("user3", "password3", session3, seq3++, "/user3/mine", 'd');

    // create a file that is owned by user3 in user4's directory.
    // should fail
    fs_create("user3", "password3", session3, seq3++, "/user4/imasneak", 'd');

    // create a file that is owned by user4
    fs_create("user4", "password4", session4, seq4++, "/user4/nomine", 'd');

    // create a file that is owned by user4 in user3's directory
    // should fail
    fs_create("user4", "password4", session4, seq4++, "/user4", 'd');

}
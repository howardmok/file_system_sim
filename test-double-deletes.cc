/*
 * test makes and deletes a bunch of files/directories. Will test deletes (and creates )
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session, seq=0;

    // const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

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

    fs_create("user1", "password1", session, seq++, "/file0", 'f');

    fs_create("user1", "password1", session, seq++, "/dir0",'d');

    fs_create("user1", "password1", session, seq++, "/dir1",'d');

    fs_create("user1", "password1", session, seq++, "/dir2",'d');

    fs_create("user1", "password1", session, seq++, "/dir3",'d');

    // ERROR: nonempty directory!
    fs_delete("user1", "password1", session, seq++, "/dir1");

    // ERROR: nonempty directory!
    fs_delete("user1", "password1", session, seq++, "/dir2");

    // delete empty file
    fs_delete("user1", "password1", session, seq++, "/file0");

    // ERROR: double delete file0!
    fs_delete("user1", "password1", session, seq++, "/file0");

    fs_create("user1", "password1", session, seq++, "/file0", 'f');

    // ERROR: delete something that isn't there
    fs_delete("user1", "password1", session, seq++, "/file0/file1");
} // end main()
/*
 * test makes and deletes a bunch of files/directories. will cause the FS to change the direntries around a bunch
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

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

    // created inodes: 1
    fs_create("user1", "password1", session, seq++, "/inode0", 'f');

    // write to the new file with write data
    fs_writeblock("user1", "password1", session, seq++, "/inode0", 0, writedata);

    // created inodes: 2
    fs_create("user1", "password1", session, seq++, "/inode1", 'd');

    // created inodes: 3
    fs_create("user1", "password1", session, seq++, "/inode2", 'd');

    // created inodes: 4
    fs_create("user1", "password1", session, seq++, "/inode3", 'd');

    // created inodes: 5
    fs_create("user1", "password1", session, seq++, "/inode4", 'd');

    // created inodes: 6
    fs_create("user1", "password1", session, seq++, "/inode5", 'd');

    // created inodes: 7
    fs_create("user1", "password1", session, seq++, "/inode6", 'd');

    // created inodes: 8
    fs_create("user1", "password1", session, seq++, "/inode7", 'd');

    //created inodes; 7
    fs_delete("user1", "password1", session, seq++, "/inode0");

    // created inodes: 8
    fs_create("user1", "password1", session, seq++, "/inode0", 'f');

    // created inodes: 9
    fs_create("user1", "password1", session, seq++, "/inode8", 'd');

    // created inodes: 10
    fs_create("user1", "password1", session, seq++, "/inode9", 'd');

    // created inodes: 11
    fs_create("user1", "password1", session, seq++, "/inode10", 'd');

    // created inodes: 12
    fs_create("user1", "password1", session, seq++, "/inode11", 'd');

    // created inodes: 13
    fs_create("user1", "password1", session, seq++, "/inode12", 'd');

    // created inodes: 14
    fs_create("user1", "password1", session, seq++, "/inode13", 'd');

    // created inodes: 15
    fs_create("user1", "password1", session, seq++, "/inode14", 'd');

    // created inodes: 16
    fs_create("user1", "password1", session, seq++, "/inode15", 'd');

    // created inodes: 17
    fs_create("user1", "password1", session, seq++, "/inode16", 'd');

    // created inodes: 18
    fs_create("user1", "password1", session, seq++, "/inode17", 'd');

    // created inodes: 19
    fs_create("user1", "password1", session, seq++, "/inode18", 'd');

    // created inodes: 20
    fs_create("user1", "password1", session, seq++, "/inode19", 'd');

    // created inodes: 21
    fs_create("user1", "password1", session, seq++, "/inode20", 'd');

    // created inodes: 22
    fs_create("user1", "password1", session, seq++, "/inode21", 'd');

    // created inodes: 23
    fs_create("user1", "password1", session, seq++, "/inode22", 'd');

    // created inodes: 24
    fs_create("user1", "password1", session, seq++, "/inode23", 'd');

    //created inodes; 23
    fs_delete("user1", "password1", session, seq++, "/inode0");

    //created inodes; 22
    fs_delete("user1", "password1", session, seq++, "/inode1");

    //created inodes; 21
    fs_delete("user1", "password1", session, seq++, "/inode2");

    //created inodes; 20
    fs_delete("user1", "password1", session, seq++, "/inode3");

    //created inodes; 19
    fs_delete("user1", "password1", session, seq++, "/inode4");

    //created inodes; 18
    fs_delete("user1", "password1", session, seq++, "/inode5");

    //created inodes; 17
    fs_delete("user1", "password1", session, seq++, "/inode6");

    //created inodes; 16
    fs_delete("user1", "password1", session, seq++, "/inode7");

    // created inodes: 16->1
    fs_create("user1", "password1", session, seq++, "/inode23/yello", 'd');
}
/*
 * invalid sequence # test
 * sequence #'s are unique and should be strictly increasing'
 */

#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session1, session2, session3, seq=0;


    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying";
    const char *writedata2 = "its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla venenatis mollis lacinia. Pellentesque tincidunt, quam eu condimentum fermentum, libero felis vehicula velit, sollicitudin laoreet sapien nisl id lorem. Vivamus sapien sem, sollicitudin sit amet blandit nec, interdum at erat. Nulla vel hendrerit ipsum, vitae blandit mauris. Nulla odio mi, venenatis sed.";

    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        // Error checking to make sure user passes in 2 additional arguments
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }

    server = argv[1]; // name of the server
    server_port = atoi(argv[2]); // port number server is filling out requests

    // init client with name of server and port number
    fs_clientinit(server, server_port); 

    // create 3 user sessions 
    fs_session("user1", "password1", &session1, seq++);
    fs_session("user2", "password2", &session2, seq++);
    fs_session("user3", "password3", &session3, seq++);

    // have user1 create a new directory
    fs_create("user1","password1", session1, seq++, "/dir", 'd');
    fs_create("user1","password1", session1, seq++, "/dir/lib", 'd');
    fs_create("user1","password1", session1, seq++, "/dir/lib/stuff.txt", 'f');
    // at the same time, user 3 will create new directories
    fs_create("user3","password3", session3, seq++, "/user3",'d');
    fs_create("user3","password3", session3, seq++, "/user3/file",'f');

    fs_writeblock("user3","password3", session3, seq++, "/user3/file", 0, writedata2);

   //  // error! block does not yet exist
    fs_writeblock("user3","password3", session3, seq++, "/user3/file", 15, writedata2);

    // try a read.. should fail!
    fs_readblock("user3", "password3", session3, seq++, "/user3/file", 15, readdata);

    // try a read.. should work!
    fs_readblock("user3", "password3", session3, seq++, "/user3/file", 0, readdata);

    // error! user2 tries to read user3's file! 
    fs_readblock("user2", "password2", session2, seq++, "/user3/file", 0, readdata);

    // incorrect session
    fs_writeblock("user1","password1", session3, seq++, "/user3/file", 0, writedata);

    // user1 writes to stuff
    fs_writeblock("user1","password1", session1, seq++, "/dir/lib/stuff.txt", 0, writedata);

    // grow the file!
   fs_writeblock("user1","password1", session1, seq++, "/dir/lib/stuff.txt", 1, writedata2);

   // user1 and user3 read.. these should not block!
   fs_readblock("user1", "password1", session1, seq++, "/dir/lib/stuff.txt", 0, readdata);
   fs_readblock("user3", "password3", session3, seq++, "/user3/file", 0, readdata);
   fs_readblock("user1", "password1", session1, seq++, "/dir/lib/stuff.txt", 1, readdata);
   fs_create("user1","password1", session1, seq++, "/dir/lib/stuffs.txt", 'f');

   // these 2 should not block each other
   fs_readblock("user1", "password1", session1, seq++, "/dir/lib/stuff.txt", 0, readdata);
   fs_writeblock("user1","password1", session1, seq++, "/dir/lib/stuffs.txt", 0, writedata);
   // return 0;
}
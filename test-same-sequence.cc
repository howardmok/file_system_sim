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
    unsigned int session1, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

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

    fs_session("user1", "password1", &session1, seq);

    // ERR: same sequence number as previous request
    fs_session("user12","password1", &session1,seq);

    // create a new directory under root named dir 
    // the arguments will be converted to a c-string under the format:
    // <session> <sequence> <pathname> <type><NULL>
    fs_create("user1", "password1", session1, seq++, "/dir", 'd');

    // create a new file under the newly created directory, /dir. 
    fs_create("user1", "password1", session1, seq++, "/dir/file", 'f');

    // write to the new file with write data
    fs_writeblock("user1", "password1", session1, seq++, "/dir/file", 0, writedata);
    
    // read from that file
    fs_readblock("user1", "password1", session1, seq++, "/dir/file", 0, readdata);

    // delete the file
    fs_delete("user1", "password1", session1, seq++, "/dir/file");
    
    seq = 0;

    // ERROR: sequence # was already used
    fs_delete("user1", "password1", session1, seq, "/dir");

    seq = 500;
    fs_delete("user1", "password1", session1, seq, "/dir");


}
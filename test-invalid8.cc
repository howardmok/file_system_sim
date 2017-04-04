/*
 * simple spec test given by spec to illustrate client-side behavior
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session, seq=0;

    const char *writedata = "LOL We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    const char *writedata2 = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.5902185092185";

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

    // create a user session. note that session is uninitialized and will receive a value from the server.
    // pass in seq=0 and then increment seq by one
    fs_session("user1", "password1", &session, seq++);

    // create a new directory under root named dir 
    // the arguments will be converted to a c-string under the format:
    // <session> <sequence> <pathname> <type><NULL>
    fs_create("user1", "password1", session, seq++, "/dir", 'd');

    // create a new file under the newly created directory, /dir. 
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    fs_create("user1", "password1", session, seq++, "/dir/file1", 'f');

    // write to the new file with write data, should only read first 512 bytes
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata2);

    // write to the new file with write data, should only read first 512 bytes
    fs_writeblock("user1", "password1", session, seq++, "/dir/file1", 0, writedata2);

    // read from that file
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);

    // write to the old file1 with new data
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    //shouldnt happen?
    if (readdata[0] == writedata[0]) {
        cout << "Threading issues, why..." << endl;
    }
    
    // invalid because nothing in block?
    fs_readblock("user1", "password1", session, seq++, "/dir/file1", 1, readdata);
}
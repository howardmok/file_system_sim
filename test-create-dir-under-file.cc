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
    fs_create("user1", "password1", session, seq++, "/file", 'f');

    // create a new file under the newly created directory, /dir. 
    fs_create("user1", "password1", session, seq++, "/file/file", 'f');
}
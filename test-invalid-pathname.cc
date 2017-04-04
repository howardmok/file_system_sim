/*
 * tests 
 */
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;
// user1 has no idea whats hes doing / he's malicious. 
int main(int argc, char *argv[]) {
    char *server;
    int server_port;
    unsigned int session3, session4, seq3=0, seq4 = 0;

    if (argc != 3) {
        // Error checking to make sure user passes in 2 additional arguments
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }

    server = argv[1]; // name of the server
    server_port = atoi(argv[2]); // port number server is filling out requests

    // init client with name of server and port number
    fs_clientinit(server, server_port);

    // creates a session for username3
    fs_session("user1", "password1", &session3, seq3++);

    // creates a session for username4
    fs_session("user2", "password2", &session4, seq4++);

    cout << "fs_sessions for user1 and user 2 should be done" << endl;

    std::string invalid_path1 = "/user";
    invalid_path1 += '\0';
    invalid_path1 += '1';

    // create a directory that is owned by user1
    fs_create("user1", "password1", session3, seq3++, invalid_path1.c_str(), 'd');

    cout << "ERROR: last request should've been an ERROR??? Maybe not anymore??" << endl;
    cout << "Creating user2 /user1 directory." << endl;
    // should not error here
    fs_create("user2", "password2", session4, seq4++, "/user1", 'd');

    cout << "user1 deleting user2's /user1 directory." << endl;

    // delete a file that is not owned by you
    fs_delete("user1", "password1", session3, seq3++, "/user1");

    cout << "ERROR: last request should've been an ERROR." << endl;

    cout << "User1 is doing something funky again." << endl;
    std::string invalid_path = "/user3/min";
    invalid_path += '\0';
    invalid_path += 'e';
    fs_create("user1", "password1", session3, seq3++, invalid_path.c_str(), 'd');

    cout << "ERROR: last request should've been an ERROR." << endl;

    // create a directory and file that is owned by username3 in username4's directory.
    // should fail
    fs_create("user1", "password1", session3, seq3++, "/user1/imasneak", 'd');
    cout << "ERROR: last request should've been an ERROR." << endl;


    cout << "WRONG SESSION #.. im using user2's lol" << endl;
    fs_create("user1", "password1", session4, seq4++, "/user4/nomine", 'f');
    cout << "ERROR: last request should've been an ERROR." << endl;


    cout << "calling the last FS_CREATE.." << endl;
    // create a file that is owned by username4 in username3's directory
    // should fail

    cout << "rats! got caught.. now let's try again, heh." << endl;
    fs_create("user1", "password1", session3, seq3++, "/user4/nomine", 'f');
    cout << "ERROR: last request should've been an ERROR." << endl;


    fs_create("user2", "password2", session4, seq4++, "/user1/user2", 'd');
}

// The correct file system should have /user1/user2 owned by user2 at the end and user1 has nothing.
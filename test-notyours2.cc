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
    unsigned int session, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    const char *writeMoreData = "Supplied directly pleasant we ignorant ecstatic of jointure so if. These spoke house of we. Ask put yet excuse person see change. Do inhabiting no stimulated unpleasing of admiration he. Enquire explain another he in brandon enjoyed be service. Given mrs she first china. Table party no or trees an while it since. On oh celebrated at be announcing dissimilar insipidity. Ham marked engage oppose cousin ask add yet.Pleased him another was settled for. Moreover end horrible endeavor entrance any families. Income appear extent on of thrown in admire. Stanhill on we if vicinity material in. Saw him smallest you provided ecstatic supplied. Garret wanted expect remain as mr. Covered parlors concern we express in visited to do. Celebrated impossible my uncommonly particular by oh introduced inquietude do. Mr oh winding it enjoyed by between. The servants securing material goodness her. Saw principles themselves ten are possession. So endeavor to continue cheerful doubtful we to. Turned advice the set vanity why mutual. Reasonably if conviction on be unsatiable discretion apartments delightful. Are melancholy appearance stimulated occasional entreaties end. Shy ham had esteem happen active county. Winding morning am shyness evident to. Garrets because elderly new manners however one village shejaslkdaskdpwuiejqwkejoiajs uaiosd uaisu ddoiqweoijqoiqi ias jijaio jqw aoj ioqwj ioqj wos";
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

    // creates a session for user1
    fs_session("user1", "password1", &session, seq++);

    // create "/dir"
    fs_create("user1", "password1", session, seq++, "/dir", 'd');

    // create "/dir/cat1"
    fs_create("user1", "password1", session, seq++, "/dir/cat1", 'f');

    // delete "/dir/cat1"
    fs_delete("user1", "password1", session, seq++, "/dir/cat1");

    // should fail to create
    // parent directory was already deleted
    fs_create("user1", "password1", session, seq++, "/dir/cat1/me", 'd');

    // should fail. already deleted it
    fs_delete("user1", "password1", session, seq++, "/dir/cat1");

    // should fail. it's been deleted already. cant read something that isnt there
    fs_readblock("user1", "password1", session, seq++, "/dir/cat1", 0, readdata);

    // creates a new file to read
    fs_create("user1", "password1", session, seq++, "/dir/readme", 'f');

    fs_writeblock("user1", "password1", session, seq++, "/dir/readme", 0, writedata);

    fs_readblock("user1", "password1", session, seq++, "/dir/readme", 0, readdata);

    for (int i = 0; i < 100; ++i) {
        cout << readdata[i];
    }
    cout << endl;

    // should fail. it hasn't been written yet.
    fs_readblock("user1", "password1", session, seq++, "/dir/readme", 1, readdata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/readme", 2, writeMoreData);
}
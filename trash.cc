/*
 * test if fs is writing to a directory
 */
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <string.h>

using namespace std;

void err_cout(string bad) {
	cout << bad << endl;
}

bool test_bad_path(string pathname) {
    unsigned int stringLoc = 0;
    string x;
    size_t pos;
    cout << endl;
    while(1) {
        pos = pathname.find('/', stringLoc);
        if (pos == string::npos) {
            return false;
        }
        x = pathname.substr(stringLoc, pos - stringLoc);
        if (x.length() > 59) { //too long
            return true;
        }
        stringLoc = pos + 1;
    }
}

int main() {
/*
CHEATSHEET:
0: message: FS_SESSION <session> <sequence><NULL>
    response: <session> <sequence><NULL>

1: message: FS_CREATE <session> <sequence> <pathname> <type><NULL>
    response: <session> <sequence><NULL>
    
2: message: FS_DELETE <session> <sequence> <pathname><NULL>
    response: <session> <sequence><NULL>

3: message: FS_READBLOCK <session> <sequence> <pathname> <block><NULL>
    response: <session> <sequence><NULL><data>

4: message: FS_WRITEBLOCK <session> <sequence> <pathname> <block><NULL><data>
    response: <session> <sequence><NULL>
*/
    string pathname = "/sjdhakjsdasjndjnasdjkaksndkasndkjanskjdajksndjasdjknasjkdasjkndkjasdjnasjkdna/what"; // should fail
    string pathname1 = "/dir/what/who"; // should pass
    string pathname2 = "/dir/abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh/who"; // should fail
    string pathname3 = "/dir/abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefg/who"; // should pass
    string pathname4 = "/dir/abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef/who"; // should pass
    cout << pathname << ": " << test_bad_path(pathname) << endl;
    cout << pathname1 << ": " << test_bad_path(pathname1) << endl;
    cout << pathname2 << ": " << test_bad_path(pathname2) << endl;
    cout << pathname3 << ": " << test_bad_path(pathname3) << endl;
    cout << pathname4 << ": " << test_bad_path(pathname4) << endl;
}
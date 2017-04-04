#include <iostream>
#include <string>
#include <cctype>

using namespace std;

int main() {
    string s0("A\0QE");
    cout << "s0: " << s0 << " len: " << s0.length() << endl;
    string s1(s0);
    cout << "s1: " << s1 << " len: " << s1.length() << endl;
    string s2(s0.c_str(), 4);
    cout << "s2: " << s2 << " len: " << s2.length() << endl;
    string s3 = "";
    s3 += 'A';
    s3 += '\0';
    s3 += 'Q';
    s3 += 'E';

    string s4 = "Hello\0there.";
    const char * s45 = s4.c_str(); 
    for (unsigned int i = 0; i < s4.size(); i++) {
        if (s45[i] == '\0'){
            cout << "<NULL>";
        }
        else {
            cout << s45[i];
        }
    }
    cout << endl;

    const char * s5 = s3.c_str();
    const char * s6 = s5;
    for(unsigned i = 0; i < s3.size(); ++i) {
        if(*s6 == '\0'){
            cout << "<NULL>";
        }
        else {
            cout << *s6;
        }
        ++s6;
    }
    cout << endl;


    string tester = "     7 6 A---- ";
    unsigned int what;
    try {
        what = stoul(tester);
    } catch (...) {
        what = 99999999;
    }

    for (int i = 0; i < tester.size(); i++) {
        if (isdigit(tester[i])) {
            cout << tester[i] << " ";
        }
    }
    cout << endl;
    cout << "what:" << what << endl;
}
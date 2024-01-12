#include "iofile.h"

using namespace std;

string &removespace(string &s)
{
    size_t len = s.length();
    int t = 0;
    for (int i = 0; i < len; ++i) {
        if (s[i]==' ' || s[i] == '\t') continue;
        s[t++] = s[i];
    }
    s.resize(t);
    return s;
}

int parseline(string s, string &lhs, string &rhs)
{
    lhs = "";
    rhs = "";
    size_t partition = s.find("#");
    if (partition != string::npos) {
        s.resize(partition);
    }

    partition = s.find("//");
    if (partition != string::npos) {
        s.resize(partition);
    }

    partition = s.find_first_of('=');

    if (partition == string::npos) {
        return -1;
    }

    lhs = s.substr(0, partition - 1);
    rhs = s.substr(partition+1);
    removespace(lhs);
    return 0;
}

void CheckFile(bool iFile, const string& filename)
{
    if (!iFile) {
        cerr << "Error: Cannot open file " << filename << "!" << endl;
        exit(2);
    }
}

int CountOperands(const string& rhs) {
    int count = 0;
    for (char i : rhs) {
        if (i == '&' || i == '|' || i == '^') {
            count++;
        }
    }
    return count;
}

void CheckOperand(node& a) {
    if (a.name[0] == '~') {
        a.inv = true;
        a.name = a.name.substr(1);
    }
    if (a.name[0] != 'n') {
        a.leaf = true;
    }
}


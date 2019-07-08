//
// Created by lwh on 19-7-9.
//

#include <iostream>
#include <sstream>
#include <chrono>
#include <regex>
#include <iterator>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include "tracer.h"
#include "lock_free_hash_table.h"
#include "concurrent_hash_table.h"
#include "basic_hash_table.h"
#include "atomic_shared_ptr.h"

using namespace std;

char *dummy[]{
        "abwerewrsdfrwerewererw",
        "werweasdrewrewweserewwwwer",
        "werqwersaserwserewwsserwew",
        "werewrserweesrttasswewrews"
};

vector<string> input;

neatlib::BasicHashTable<char *, char *, std::hash<char *>, std::equal_to<char *>, std::allocator<std::pair<const char *, char *>>, 4> *mhash;

void simpleInsert() {
    for (int i = 0; i < 4; i++) {
        mhash->Insert(dummy[i], dummy[i]);
    }
    for (int i = 0; i < 4; i++) {
        cout << mhash->Get(dummy[i]).second << endl;
    }
}

void initYCSB(char *path, size_t count) {
    ifstream fin(path);
    string s;
    string regex_str("user\\d");
    regex pattern(regex_str, regex::icase);
    match_results<string::const_iterator> results;
    long found = 0;
    while (getline(fin, s)) {
        if (s.substr(0, 6) == "INSERT") {
            if (regex_search(s, results, pattern)) {
                size_t spos = results[0].second - s.begin();
                size_t epos = s.find(" ", spos);
                string os = s.substr(spos - 1, epos - spos + 1);
                input.push_back(os);
                found++;
                //cout << found++ << " " << os << " " << os.size() << endl;
            }
            //cout << s << endl;
            /*size_t spos = s.find("user\\[0-9\\]");
            size_t epos = s.find(" ", spos);
            cout << s.substr(spos + 4, epos) << endl;*/
        }
    }
    cout << found << endl;
}

int main(int argc, char **argv) {
    mhash = new neatlib::BasicHashTable<char *, char *, std::hash<char *>, std::equal_to<char *>, std::allocator<std::pair<const char *, char *>>, 4>();
    if (argc <= 1) {
        cout << "Parameter needed." << endl;
        exit(0);
    }
    switch (std::atoi(argv[1])) {
        case 0:
            simpleInsert();
            break;
        case 1:
            initYCSB(argv[2], std::atol(argv[3]));
            break;
        default:
            cout << "Parameter needed." << endl;
    }
    return 0;
}
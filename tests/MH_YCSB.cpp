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
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
};


char *update[]{
        "werewsdvcxewwrsdfxcvwerwesdfwerw",
        "werewsdvcxewwrsdfxcvwerwesdfwerw"
        "werewsdvcxewwrsdfxcvwerwesdfwerw"
        "werewsdvcxewwrsdfxcvwerwesdfwerw"
        "werewsdvcxewwrsdfxcvwerwesdfwerw",
        "werewsdvcxewwrsdfxcvwerwesdfwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "werewsdvcxewwrsdfxcvwerwesdfwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "werewsdvcxewwrsdfxcvwerwesdfwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
};

//vector<string> input;

size_t total = 100;

char **sinput;

neatlib::BasicHashTable<char *, char *, std::hash<char *>, std::equal_to<char *>, std::allocator<std::pair<const char *, char *>>, 4> *mhash;

void simpleInsert() {
    for (int i = 0; i < 4; i++) {
        mhash->Insert(dummy[i], dummy[i]);
    }
    for (int i = 0; i < 4; i++) {
        cout << mhash->Get(dummy[i]).second << endl;
    }
}

void loadYCSB(char *path) {
    sinput = new char *[total];
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
                sinput[found] = new char[os.size() + 1];
                mempcpy(sinput[found++], os.c_str(), os.size());
                //input.push_back(os);
                //cout << found++ << " " << os << " " << os.size() << endl;
            }
            //cout << s << endl;
            /*size_t spos = s.find("user\\[0-9\\]");
            size_t epos = s.find(" ", spos);
            cout << s.substr(spos + 4, epos) << endl;*/
        }
        if (found == total) break;
    }
    cout << found << endl;
}

void initYCSB(int vscale) {
    for (int i = 0; i < total; i++) {
        mhash->Insert(sinput[i], dummy[vscale]);
    }
}

void freeLoad() {
    for (int i = 0; i < total; i++) {
        delete[] sinput[i];
    }
    delete[] sinput;
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
            Tracer tracer;
            tracer.startTime();
            total = std::atol(argv[3]);
            loadYCSB(argv[2]);
            cout << "Load time: " << tracer.getRunTime() << endl;
            tracer.startTime();
            initYCSB(std::atoi(argv[4]));
            cout << "Init time: " << tracer.getRunTime() << endl;
            freeLoad();
            break;
        default:
            cout << "Parameter needed." << endl;
    }
    return 0;
}
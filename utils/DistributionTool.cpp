//
// Created by lwh on 19-6-16.
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <iterator>

using namespace std;

enum COM {
    READ,
    INSERT,
    UPDATE,
    DELETE
};

string TYPES[] = {"READ", "INSERT", "UPDATE", "DELETE"};

vector<pair<string, uint64_t>> orderedCounters(COM com, char *inpath) {
    unordered_map<string, uint64_t> counters;
    ifstream fin(inpath);
    string line;
    string type;
    string content;
    while (getline(fin, line)) {
        stringstream ss(line);
        getline(ss, type, ' ');
        if (type == TYPES[com]) {
            getline(ss, content, ' ');
            if (counters.find(content) != counters.end()) {
                counters.find(content)->second++;
            } else {
                counters.insert(make_pair(content, 1));
            }
            //cout << exists << " " << content << endl;
        }
    }
    vector<pair<string, uint64_t>> tmp;
    for (auto &e: counters) {
        tmp.push_back(e);
    }
    sort(tmp.begin(), tmp.end(),
         [=](pair<string, uint64_t> &a, pair<string, uint64_t> &b) { return b.second < a.second; });
    vector<pair<string, uint64_t>>::iterator iter = tmp.begin();
    while (iter++ != tmp.end()) {
        if (iter - tmp.begin() == 1000)
            break;
        cout << iter->first << " " << iter->second << endl;
    }
    /*map<int, string> tmp;
    transform(counters.begin(), counters.end(), std::inserter(tmp, tmp.begin()),
              [](std::pair<string, int> a) { return std::pair<int, string>(a.second, a.first); });

    map<int, string>::iterator iter = tmp.begin();
    while (++iter != tmp.end()) {
        cout << (*iter).first << " " << (*iter).second << endl;
    }*/
    cout << counters.size() << "<->" << tmp.size() << endl;
}

int main(int argc, char **argv) {
    char *inpath = argv[1];
    orderedCounters(READ, inpath);
}
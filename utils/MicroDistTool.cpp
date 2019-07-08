//
// Created by lwh on 19-7-8.
//

#include <stdio.h>
#include <iostream>
#include <unordered_set>

using namespace std;

enum StatType {
    UNIQUE_COUNT = 0,
    DEFAULT_COUNT
};

StatType type = UNIQUE_COUNT;

char *path = "./testfile.dat";

size_t count = 1000000000;

void uniqueCount() {
    uint64_t *array = new uint64_t[count];
    FILE *fp = fopen(path, "rb+");
    fread(array, sizeof(size_t), count, fp);
    fclose(fp);
    unordered_set<uint64_t> uniqueSet;
    for (size_t i = 0; i < count; i++) {
        uniqueSet.insert(array[i]);
    }
    cout << "Total unique " << uniqueSet.size() << " out of " << count << endl;
    delete[] array;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        type = static_cast<StatType >(std::atoi(argv[1]));
        path = argv[2];
        count = std::atol(argv[3]);
    }
    switch (type) {
        case UNIQUE_COUNT:
            uniqueCount();
            break;
        default:
            cout << "Type: 0 (unique counter)" << endl;
    }
    return 0;
}
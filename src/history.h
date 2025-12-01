#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

using namespace std;

class History {
private:
    static std::vector<string> history;
    static std::size_t lastFlushedIdx;

public:
    static void add(const string& cmd);

    static void handle(const string& payload);
};

#endif

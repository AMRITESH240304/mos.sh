#ifndef ENVCACHE_H
#define ENVCACHE_H

#include <string>
#include <unordered_map>
#include <vector>
#include "utils.h"

using namespace std;

class EnvCache {
public:
    static string homeDir;
    static vector<string> pathDirs;
    static void initialize();
    static string findExecutable(const string& command);

};

#endif // ENVCACHE_H
#include <string>
#include <vector>
#include <unistd.h>
#include "env.h"
#include "utils.h"

using namespace std;

string EnvCache::homeDir = "";
vector<string> EnvCache::pathDirs = {};

void EnvCache::initialize() {
    const char* home = getenv("HOME");
    homeDir = home ? string(home) : "";

    const char* path = getenv("PATH");
    if (path) {
        pathDirs = split(string(path), ':'); 
    }
}

string EnvCache::findExecutable(const string& command) {
    if (command.find('/') != string::npos) {
        return command; 
    }

    for (const auto& dir : EnvCache::pathDirs) {
        string fullPath = dir + "/" + command;
        if (access(fullPath.c_str(), X_OK) == 0) {
            return fullPath;
        }
    }
    return "";
}


#include "commands.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>

using namespace std;

void CommandHandler::handleEcho(const string& args) {
    cout << args << endl;
}

vector<string> split(string str, char delimiter = ':') {
    vector<string> result;
    string token = "";

    for (auto i:str) {
        if (i == delimiter) {
            result.push_back(token);
            token = "";
        }
        else {
            token += i;
        }
    }

    if (!token.empty()) {
        result.push_back(token);
    }

    return result;
}

void CommandHandler::handleType(const string& args, const vector<string>& builtins) {
    if (isBuiltin(args, builtins)) {
        cout << args << " is a shell builtin" << endl;
    } else {
        bool found = false;
        const char* pathenv = getenv("PATH");
        if (pathenv) {
            vector<string> paths = split(pathenv);
            for (auto i:paths) {
                string fullpath = i + "/" + args;
                if (access(fullpath.c_str(), X_OK) == 0) {
                    cout << args << " is " << fullpath << endl;
                    found = true;
                    break;
                }
            }
        } else {
            cout << "PATH not set" << endl;
        }
        if (!found){
            cout << args << ": not found" << endl;
            return;
        }
    }
}

bool CommandHandler::isBuiltin(const string& command, const vector<string>& builtins) {
    return find(builtins.begin(), builtins.end(), command) != builtins.end();
}
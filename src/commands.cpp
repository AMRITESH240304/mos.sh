#include "commands.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

vector<string> split_args(string str, char delimiter = ' ') {
    vector<string> result;
    string token = "";
    for (auto i:str) {
        if (i == delimiter) {
            if (!token.empty()) {
                result.push_back(token);

                token = "";
            }
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

void CommandHandler::externalProgram(const string& command) {
    const char* pathenv = getenv("PATH");
    if (pathenv) {
        vector<string> paths = split(pathenv);
        vector<string> args = split_args(command);
        
        vector<char*> argv;

        for (auto& arg : args){
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
         
        argv.push_back(nullptr);  

        for (auto dir : paths) {
            string fullpath = dir + "/" + args[0];
            if (access(fullpath.c_str(), X_OK) == 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    execv(fullpath.c_str(), argv.data());
                    perror("execv failed");
                    exit(1);
                } else if (pid > 0) {
                    int status;
                    waitpid(pid, &status, 0);
                    return;
                } else {
                    perror("fork failed");
                    return;
                }
            }
        }
    }
    throw runtime_error("Command not found");
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

void CommandHandler::handleAboslutePath(const string& path) {

    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            string newPath = string(home) + path.substr(1);
            if (chdir(newPath.c_str()) != 0) {
                std::perror("cd failed");
            }
            return;
        }
    }

    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        std::cout << "cd: " << path << ": No such file or directory" << std::endl;
        return;
    }

    if (chdir(path.c_str()) != 0) {
        std::perror("cd failed");
        return;
    }
}

void CommandHandler::handlePwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        cout << cwd << endl;
    } else {
        perror("getcwd() error");
    }
}

bool CommandHandler::isBuiltin(const string& command, const vector<string>& builtins) {
    return find(builtins.begin(), builtins.end(), command) != builtins.end();
}
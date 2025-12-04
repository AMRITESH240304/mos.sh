#include "commands.h"
#include "../utils/utils.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <cstdio>
#include "../utils/env.h"

using namespace std;

void CommandHandler::handleEcho(const string& args) {
    vector<string> tokens = split_args(args);
    
    for (size_t i = 0; i < tokens.size(); i++) {
        cout << tokens[i];
        if (i < tokens.size() - 1) {
            cout << " ";
        }
    }
    cout << endl;
}

void CommandHandler::handleCat(const string& filePath) {
    vector<string> result = split_args(filePath);

    for (auto& file : result) {
        ifstream infile(file);
        if (!infile) {
            cerr << "cat: " << file << ": No such file or directory" << endl;
            continue;
        }

        stringstream buffer;
        buffer << infile.rdbuf();
        cout << buffer.str();
        infile.close();
    }
}

void CommandHandler::handlePipeline(const std::string &left, const std::string &right) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Child 1 — producer (left command)
        close(pipefd[0]);                // Close read end
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout → pipe
        close(pipefd[1]);

        execlp("sh", "sh", "-c", left.c_str(), NULL);
        perror("execlp");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Child 2 — consumer (right command)
        close(pipefd[1]);                // Close write end
        dup2(pipefd[0], STDIN_FILENO);   // Redirect stdin ← pipe
        close(pipefd[0]);

        execlp("sh", "sh", "-c", right.c_str(), NULL);
        perror("execlp");
        exit(1);
    }

    // Parent closes both ends
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both commands
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


void CommandHandler::externalProgram(const string& command) {
    vector<string> args = split_args(command);
    if (args.empty()) return;

    string fullPath = EnvCache::findExecutable(args[0]);

    if (fullPath.empty()) {
        throw runtime_error("Command not found");
    }

    vector<char*> argv;
    for (auto& arg : args){
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execv(fullPath.c_str(), argv.data());
        
        // If execv returns, it failed
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}

void CommandHandler::handleType(const string& args, const vector<string>& builtins) {
    if (isBuiltin(args, builtins)) {
        cout << args << " is a shell builtin" << endl;
    } else {
        string fullPath = EnvCache::findExecutable(args);
        
        if (!fullPath.empty()) {
            cout << args << " is " << fullPath << endl;
        } else {
            cout << args << ": not found" << endl;
        }
    }
}

void CommandHandler::handleNavigation(const string& path) {

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

bool CommandHandler::HandleRedirections(const ParsedCommand& parsed, int& savedStdout) {
    if (!parsed.redirect) {
        return false;
    }

    if (parsed.outputFile.empty()) {
        cerr << "redirect: missing output file" << endl;
        return false;
    }

    if (parsed.redirectFd != STDOUT_FILENO && parsed.redirectFd != STDERR_FILENO) {
        cerr << "redirect: unsupported file descriptor " << parsed.redirectFd << endl;
        return false;
    }

    int flags = O_WRONLY | O_CREAT;

    if (parsed.append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;

    int targetFd = open(parsed.outputFile.c_str(), flags, 0644);
    if (targetFd == -1) {
        perror("open");
        return false;
    }

    int saved = dup(parsed.redirectFd);
    if (saved == -1) {
        perror("dup");
        close(targetFd);
        return false;
    }

    cout.flush();
    cerr.flush();
    fflush(stdout);
    fflush(stderr);

    if (dup2(targetFd, parsed.redirectFd) == -1) {
        perror("dup2");
        close(targetFd);
        close(saved);
        return false;
    }

    close(targetFd);
    savedStdout = saved;
    return true;
}
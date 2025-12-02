#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>
#include "commands.h"
#include "utils.h"
#include "history.h"
#include <dirent.h>
#include <sys/stat.h>
#include "completion.h"
#include <algorithm>

using namespace std;
static vector<string> builtins = {"echo", "type", "exit", "pwd", "cd", "history"};

// tab completion reference:
// https://thoughtbot.com/blog/tab-completion-in-gnu-readline
int main() {
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    init_completion(builtins);

    // vector<string> history_vec;
    const char* histEnv = getenv("HISTFILE");
    string histPath = histEnv ? string(histEnv) : "";
    History::loadFromFile(histPath);

    while (true) {
        char* input_cstr = readline("$ ");
        if (!input_cstr) break;
        
        string input(input_cstr);
        free(input_cstr);
        
        if (!input.empty()) {
            History::add(input);
        }

        if (input == "exit 0" || input == "exit") {
            break;
        }
        
        Pipeline pip = parsePipeline(input);

        if (pip.isPipe) {
            CommandHandler::handlePipeline(pip.leftCmd, pip.rightCmd);
            continue;
        }

        ParsedCommand parsed = parseCommand(input);

        int savedStdout = -1;
        bool redirected = CommandHandler::HandleRedirections(parsed, savedStdout);

        string cmd = parsed.cmd;
        if (cmd.empty()) {
            if (redirected && savedStdout != -1) {
                dup2(savedStdout, parsed.redirectFd);
                close(savedStdout);
            }
            continue;
        }

        vector<string> args = split_args(cmd);
        string name = args.empty() ? "" : args[0];
        string payload = (args.size() > 1) ? cmd.substr(name.size() + 1) : "";

        if (name == "echo") {
            CommandHandler::handleEcho(payload);
        }
        else if (name == "type") {
            CommandHandler::handleType(payload, builtins);
        }
        else if (name == "pwd") {
            CommandHandler::handlePwd();
        }
        else if (name == "cd") {
            CommandHandler::handleNavigation(payload);
        }
        else if (name == "cat") {
            if (!payload.empty())
                CommandHandler::handleCat(payload);
        }
        else if (name == "history") {
            History::handle(payload);
        }
        else {
            try {
                CommandHandler::externalProgram(cmd);
            } catch (...) {
                cout << name << ": command not found" << endl;
            }
        }

        if (redirected && savedStdout != -1) {
            fflush(stdout);
            dup2(savedStdout, parsed.redirectFd);
            close(savedStdout);
        }
    }

    if(!histPath.empty()) {
        History::saveSnapshotToFile(histPath);
    }

    return 0;
}
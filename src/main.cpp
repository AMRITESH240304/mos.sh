#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include "commands.h"
#include "utils.h"

using namespace std;

int main() {
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    string input;
    vector<string> builtins = {"echo", "type", "exit", "pwd", "cd"};

    while (true) {
        cout << "$ ";
        if (!getline(cin, input)) break;

        if (input == "exit 0" || input == "exit") {
            break;
        }

        ParsedCommand parsed = parseCommand(input);

        int savedStdout = -1;
        bool redirected = CommandHandler::HandleRedirections(parsed, savedStdout);

        string cmd = parsed.cmd;
        if (cmd.empty()) {
            cerr << "Error: empty command\n";
            if (redirected && savedStdout != -1) {
                dup2(savedStdout, STDOUT_FILENO);
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
        else {
            try {
                CommandHandler::externalProgram(cmd);
            } catch (...) {
                cout << name << ": command not found" << endl;
            }
        }

        if (redirected && savedStdout != -1) {
            fflush(stdout);
            dup2(savedStdout, STDOUT_FILENO);
            close(savedStdout);
        }
    }

    return 0;
}

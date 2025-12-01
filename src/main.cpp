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

using namespace std;

int main() {
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    vector<string> builtins = {"echo", "type", "exit", "pwd", "cd", "history"};
    vector<string> history_vec;

    while (true) {
        char* input_cstr = readline("$ ");
        if (!input_cstr) break;
        
        string input(input_cstr);
        free(input_cstr);
        
        if (!input.empty()) {
            add_history(input.c_str());
            history_vec.push_back(input);
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
            vector<string> historyArgs = split_args(payload);
            
            if (!historyArgs.empty() && historyArgs[0] == "-r") {
                if (historyArgs.size() < 2) {
                    cout << "history: -r requires a file path" << endl;
                } else {
                    string filepath = historyArgs[1];
                    ifstream file(filepath);
                    
                    if (!file.is_open()) {
                        cout << "history: cannot read " << filepath << endl;
                    } else {
                        string line;
                        while (getline(file, line)) {
                            if (!line.empty()) {
                                add_history(line.c_str());
                                history_vec.push_back(line);
                            }
                        }
                        file.close();
                    }
                }
            } else {
                int n = history_vec.size();

                if (!payload.empty() && historyArgs[0] != "-r") {
                    try {
                        n = stoi(payload);
                    } catch (...) {
                        n = history_vec.size();
                    }
                }

                if (n > (int)history_vec.size()) {
                    n = history_vec.size();
                }

                int start = history_vec.size() - n;

                for (int i = start; i < (int)history_vec.size(); ++i) {
                    cout << "    " << i + 1 << "  " << history_vec[i] << endl;
                }
            }
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

    return 0;
}

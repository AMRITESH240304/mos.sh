#include <iostream>
#include <string>
#include <vector>
#include "commands.h"

using namespace std;

int main() {
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    string command;
    vector<string> builtins = {"echo", "type", "exit", "pwd", "cd"};

    while(true) {
        cout << "$ ";
        getline(cin, command);

        if (command == "exit 0") {
            break;
        }

        if (command.find("echo ") == 0 || command == "echo") {
          if (command == "echo") {
              CommandHandler::handleEcho("");
              continue;
          }
            CommandHandler::handleEcho(command.substr(5));
            continue;
        }

        if (command.find("type ") == 0) {
            CommandHandler::handleType(command.substr(5), builtins);
            continue;
        }

        if (command == "pwd") {
            CommandHandler::handlePwd();
            continue;
        }

        if (command.find("cd ") == 0 || command == "cd") {
            if (command == "cd") {
                continue;
            }
            CommandHandler::handleNavigation(command.substr(3));
            continue;
        }
        try {
            CommandHandler::externalProgram(command);
            continue;
        }
        catch (...) {
          cout << command << ": command not found" << endl;
        }
    }

    return 0;
}

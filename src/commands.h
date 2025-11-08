#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>

using namespace std;

class CommandHandler {
public:
    static void handleEcho(const string& args);
    static void handleType(const string& args, const vector<string>& builtins);
    static bool isBuiltin(const string& command, const vector<string>& builtins);
    static void externalProgram(const string& command);
};

#endif // COMMANDS_H
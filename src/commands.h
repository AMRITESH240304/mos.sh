#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include "helper.h"

using namespace std;

class CommandHandler {
public:
    static void handleEcho(const string& args);
    static void handleType(const string& args, const vector<string>& builtins);
    static bool isBuiltin(const string& command, const vector<string>& builtins);
    static void externalProgram(const string& command);
    static void handlePwd();
    static void handleNavigation(const string& path);
    static void handleCat(const string& filepath);
    static bool HandleRedirections(const ParsedCommand& parsed, int& savedStdout);
};

#endif // COMMANDS_H
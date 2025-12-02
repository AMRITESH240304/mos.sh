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
#include <algorithm>

using namespace std;
static vector<string> builtins = {"echo", "type", "exit", "pwd", "cd", "history"};

// tab completion reference:
// https://thoughtbot.com/blog/tab-completion-in-gnu-readline
static char** charater_command_completion(const char *, int, int);
static char* character_command_generator(const char *, int);
static char* external_command_generator(const char *, int);

int main() {
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    rl_attempted_completion_function = charater_command_completion;

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


static char** charater_command_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, character_command_generator);
}   

static char* character_command_generator(const char *text, int state) {
    static size_t list_index;
    static size_t len;
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    const char* name;
    while (list_index < builtins.size() && (name = builtins[list_index++].c_str())) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return external_command_generator(text, state);
}

static char* external_command_generator(const char *text, int state) {
    static std::vector<std::string> matches;
    static size_t idx;
    static bool initialized;

    if (!state) {
        matches.clear();
        idx = 0;
        initialized = false;
    }

    if (!initialized) {
        initialized = true;
        size_t len = strlen(text);
        const char* pathEnv = getenv("PATH");
        if (pathEnv) {
            std::vector<std::string> dirs = split(std::string(pathEnv)); // uses utils::split(':')
            for (const auto &d : dirs) {
                std::string dir = d.empty() ? "." : d;
                DIR *dp = opendir(dir.c_str());
                if (!dp) continue;
                struct dirent *entry;
                while ((entry = readdir(dp)) != nullptr) {
                    const char *nm = entry->d_name;
                    if (len != 0 && strncmp(nm, text, len) != 0) continue;
                    std::string full = dir + "/" + nm;
                    struct stat st;
                    if (stat(full.c_str(), &st) == 0) {
                        if ((S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) && access(full.c_str(), X_OK) == 0) {
                            matches.emplace_back(nm);
                        }
                    }
                }
                closedir(dp);
            }
        }
        std::sort(matches.begin(), matches.end());
        matches.erase(std::unique(matches.begin(), matches.end()), matches.end());
    }

    if (idx < matches.size()) {
        std::string out = matches[idx++];
        // Do NOT add a trailing space; readline will handle spacing
        // out.push_back(' ');
        return strdup(out.c_str());
    }
    return nullptr;
}
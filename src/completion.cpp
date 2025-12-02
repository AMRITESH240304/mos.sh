#include "completion.h"
#include "utils.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

static vector<string> s_builtins;

static char** shell_completion(const char *text, int start, int end);
static char* command_generator(const char *text, int state);
static char* external_command_generator(const char *text, int state);

void init_completion(const vector<string>& builtins) {
    s_builtins = builtins;
    rl_attempted_completion_function = shell_completion;
}

static char** shell_completion(const char *text, int start, int end) {
    (void)end;
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}

static char* command_generator(const char *text, int state) {
    static size_t list_index;
    static size_t len;
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    const char* name;
    while (list_index < s_builtins.size() && (name = s_builtins[list_index++].c_str())) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return external_command_generator(text, state);
}

static char* external_command_generator(const char *text, int state) {
    static vector<string> matches;
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
            vector<string> dirs = split(string(pathEnv)); 
            for (const auto &d : dirs) {
                string dir = d.empty() ? "." : d;
                DIR *dp = opendir(dir.c_str());
                if (!dp) continue;
                struct dirent *entry;
                while ((entry = readdir(dp)) != nullptr) {
                    const char *nm = entry->d_name;
                    if (len != 0 && strncmp(nm, text, len) != 0) continue;
                    string full = dir + "/" + nm;
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
        sort(matches.begin(), matches.end());
        matches.erase(unique(matches.begin(), matches.end()), matches.end());
    }

    if (idx < matches.size()) {
        const string &out = matches[idx++];
        return strdup(out.c_str());
    }
    return nullptr;
}
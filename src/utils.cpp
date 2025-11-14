#include "utils.h"
#include <cctype>

using namespace std;

vector<string> split(string str, char delimiter) {
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

vector<string> split_args(string str, char delimiter) {
    vector<string> args;
    string current;
    char quote_char = '\0';  
    bool escaped = false;

    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (c == '\\' && !escaped) {
            if (quote_char == '"') {
                char next = (i + 1 < str.size()) ? str[i+1] : '\0';
                if (next == '"' || next == '\\') {
                    escaped = true;
                    continue;
                }
                current += '\\';
                continue;
            }

            if (quote_char == '\0') {
                escaped = true;
                continue;
            }
            current += '\\';
            continue;
        }

        if (escaped) {
            current += c;
            escaped = false;
            continue;
        }

        if ((c == '\'' || c == '\"') && quote_char == '\0') {
            quote_char = c;
        } else if (c == quote_char) {
            quote_char = '\0';
        } else if (isspace(c) && quote_char == '\0') {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) args.push_back(current);
    return args;
}
#include "utils.h"
#include "helper.h"
#include <cctype>

using namespace std;

ParsedCommand parseCommand(const string& input){
    ParsedCommand pc;
    auto trim = [](std::string &s) {
        while (!s.empty() && isspace(static_cast<unsigned char>(s.front()))) s.erase(0, 1);
        while (!s.empty() && isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    };

    std::string working = input;
    trim(working);

    size_t pos = working.find(">>");
    bool isAppend = false;

    if (pos != string::npos) {
        isAppend = true;
    } else {
        pos = working.find('>');
    }

    if (pos != std::string::npos) {
        size_t fdStart = pos;
        while (fdStart > 0 && isdigit(static_cast<unsigned char>(working[fdStart - 1]))) {
            fdStart--;
        }

        bool hasExplicitFd = fdStart < pos;
        if (hasExplicitFd) {
            bool digitsOnly = true;
            for (size_t i = fdStart; i < pos; ++i) {
                if (!isdigit(static_cast<unsigned char>(working[i]))) {
                    digitsOnly = false;
                    break;
                }
            }

            if (digitsOnly && (fdStart == 0 || isspace((unsigned char)working[fdStart - 1]))) {
                pc.redirectFd = stoi(working.substr(fdStart, pos - fdStart));
            } else {
                hasExplicitFd = false;
            }

            if (!hasExplicitFd) {
                fdStart = pos;
            }
        }

        pc.redirect = true;
        pc.append = isAppend;

        pc.cmd = working.substr(0, fdStart);
        pc.outputFile = working.substr(pos + (isAppend ? 2 : 1));

        trim(pc.cmd);
        trim(pc.outputFile);
    } else {
        pc.cmd = working;
    }

    return pc;
}

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
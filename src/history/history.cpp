#include "history.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <readline/history.h>

// Define the static member

using namespace std;

vector<string> History::history;
size_t History::lastFlushedIdx = 0;

void History::add(const string& cmd) {
    if (!cmd.empty()) {
        history.push_back(cmd);
        add_history(cmd.c_str());
    }
}

void History::loadFromFile(const string& path) {
    if (path.empty()) return;
    ifstream file(path);
    if(!file.is_open()) return;
    string line;
    while(getline(file,line)) {
        if(!line.empty()) {
            history.push_back(line);
            add_history(line.c_str());
        }
    }
    file.close();

    lastFlushedIdx = history.size();
}

void History::saveSnapshotToFile(const string& path) {
    if (path.empty()) return;
    ofstream out(path, ios::out | ios::trunc);
    if(!out.is_open()) return;
    for (auto line: history){
        out << line << '\n';
    }
    out.close();
    lastFlushedIdx = history.size();
}

void History::handle(const string& payload) {
    vector<string> args;
    string word;
    stringstream ss(payload);
    while (ss >> word) args.push_back(word);

    if (!args.empty() && args[0] == "-r") {
        if (args.size() < 2) {
            cout << "history: -r requires a file path\n";
            return;
        }

        string filepath = args[1];
        ifstream file(filepath);

        if (!file.is_open()) {
            cout << "history: cannot read " << filepath << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                history.push_back(line);
                add_history(line.c_str());
            }
        }

        file.close();
        return;
    }

    if (!args.empty() && args[0] == "-w") {
        if (args.size() < 2) {
            cout << "history: -w requires a file path\n";
            return;
        }
        const string& path = args[1];
        ofstream out(path, ios::out | ios::trunc);
        if (!out.is_open()) {
            cout << "history: cannot write " << path << "\n";
            return;
        }
        for (auto line : history) {
            out << line << '\n'; 
        }
        out.close();

        lastFlushedIdx = history.size();
        return;
    }

    if (!args.empty() && args[0] == "-a") {
        if (args.size() < 2) {
            cout << "history: -a requires a file path\n";
            return;
        }
        const string& path = args[1];
        ofstream out(path, ios::out | ios::app);
        if (!out.is_open()) {
            cout << "history: cannot append to " << path << "\n";
            return;
        }
        for (size_t i = lastFlushedIdx; i < history.size(); ++i) {
            out << history[i] << '\n';
        }
        out.close();
        lastFlushedIdx = history.size();
        return;
    }

    int n = history.size();

    if (!args.empty() && args[0] != "-r") {
        try {
            n = stoi(args[0]);
        } catch (...) {
            n = history.size();
        }
    }

    if (n > (int)history.size())
        n = history.size();

    int start = history.size() - n;

    for (int i = start; i < (int)history.size(); ++i) {
        cout << "    " << i + 1 << "  " << history[i] << endl;
    }
}

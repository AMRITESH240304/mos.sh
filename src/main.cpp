#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

void echoCommand(string args);

int main() {
  // Flush after every cout / cerr
  cout << unitbuf;
  cerr << unitbuf;

  string command;
  // TODO: Uncomment the code below to pass the first stage
  vector<string> builtins = {"echo", "type", "exit"};

  while(true) {
    cout << "$ ";
    getline(cin, command);

    if (command == "exit 0") {
      break;
    }

    if (command.find("echo ") == 0) {
      echoCommand(command.substr(5));
      continue;
    }

    if (command.find("type ") == 0) {
      string arg = command.substr(5); 

      if (find(builtins.begin(), builtins.end(), arg) != builtins.end()) {
          cout << arg << " is a shell builtin" << endl;
      } else {
          cout << arg << ": not found" << endl;
      }

      continue;
    }

    cout << command << ": command not found" << endl;
  }

}

void echoCommand(string args) {
  cout << args << endl;
}

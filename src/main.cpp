#include <iostream>
#include <string>

using namespace std;

void echoCommand(string args);

int main() {
  // Flush after every cout / cerr
  cout << unitbuf;
  cerr << unitbuf;

  string command;
  // TODO: Uncomment the code below to pass the first stage

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

    cout << command << ": command not found" << endl;
  }

}

void echoCommand(string args) {
  cout << args << endl;
}

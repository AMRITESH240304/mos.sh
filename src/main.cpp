#include <iostream>
#include <string>

using namespace std;

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

    cout << command << ": command not found" << endl;
  }
    
}

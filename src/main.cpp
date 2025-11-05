#include <iostream>
#include <string>

using namespace std;

int main() {
  // Flush after every cout / cerr
  cout << unitbuf;
  cerr << unitbuf;

  string command;
  // TODO: Uncomment the code below to pass the first stage
  cout << "$ ";
  getline(cin, command);
  cout << command << ": command not found" << endl;
}

#ifndef PARSER_HELPER_H
#define PARSER_HELPER_H

#include <string>

struct ParsedCommand {
    std::string cmd;
    std::string outputFile;
    bool redirect = false;
    int redirectFd = 1;
};

ParsedCommand parseCommand(const std::string& input);

#endif

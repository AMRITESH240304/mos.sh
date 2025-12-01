#ifndef PARSER_HELPER_H
#define PARSER_HELPER_H

#include <string>

struct ParsedCommand {
    std::string cmd;
    std::string outputFile;
    bool redirect = false;
    bool append = false;
    int redirectFd = 1;
};

struct Pipeline {
    bool isPipe = false;
    std::string leftCmd;
    std::string rightCmd;
};

ParsedCommand parseCommand(const std::string& input);
Pipeline parsePipeline(const std::string& input);

#endif

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

std::vector<std::string> split(std::string str, char delimiter = ':');
std::vector<std::string> split_args(std::string str, char delimiter = ' ');

#endif
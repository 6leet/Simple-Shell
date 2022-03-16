#ifndef HANDLER
#define HANDLER

#include <string>
#include <vector>

int printenvHandler(std::string);
int setenvHandler(std::string, std::string);
int binHandler(std::vector<std::string>);
int printenvAllHandler();
void writeToFile(int readfd, std::string filename);
void redirectHandler(bool redirect, std::string filename);

#endif
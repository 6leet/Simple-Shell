#ifndef NUMPIPE
#define NUMPIPE

#include <vector>
#include <unistd.h>

struct NumPipe {
    int ttl;
    std::vector<int> pipeend;
    NumPipe(int _ttl);
    bool countdown();
    void shutdown();
};

struct NumPipeHandler {
    std::vector<NumPipe> npipes;
    NumPipeHandler();
    void push(int ttl);
    void handle(std::vector<int> &nps, bool first, bool numpipe);
    void getTimeout(std::vector<int> &nps);
    void pop(std::vector<int> &nps);
};

#endif
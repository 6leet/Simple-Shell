#ifndef PIPE
#define PIPE

#include <vector>
#include <unistd.h>

struct PipeHandler {
    std::vector<std::vector<int> > pipes;
    PipeHandler();
    bool push();
    void handle(int cmdid);
    void pop(int p);
};

struct NumPipe {
    int ttl;
    std::vector<int> pipeend;
    NumPipe(int _ttl);
    bool countdown(int _ttl);
    void shutdown();
};

struct NumPipeHandler {
    std::vector<NumPipe> npipes;
    NumPipeHandler();
    int size();
    void push(int ttl);
    void handle(std::vector<int> &nps, bool first, int numPipeType, int np);
    void getTimeoutN(std::vector<int> &nps, int ttl);
    void pop(std::vector<int> &nps);
};

#endif
#include <vector>
#include <unistd.h>
#include <iostream>
#include "pipe.h"

PipeHandler::PipeHandler() {
    pipes.clear();
    std::vector<int> emptypipe(2, -1);
    pipes.push_back(emptypipe);
}

bool PipeHandler::push() {
    int _pipe[2];
    if (pipe(_pipe) < 0) { // debug
        std::cerr << "can't create pipe\n";
        return false;
    }
    std::vector<int> pipe(std::begin(_pipe), std::end(_pipe));
    pipes.push_back(pipe);
    return true;
}

void PipeHandler::handle(int cmdid) {
    int rp = cmdid, wp = cmdid + 1;
    for (int p = cmdid; p < pipes.size(); p++) { // ugly: cmdid (how to reuse pipe prettier?)
        if (p != rp || p == 0) { // 0 is empty. must close
            // std::cout << cmdid << " close read " << pipes[p][0] << "\n";
            close(pipes[p][0]);
        }
        if (p != wp || p == 0) { // 0 is empty. must close
            // std::cout << cmdid << " close write " << pipes[p][1] << "\n";
            close(pipes[p][1]);
        }
    }
    if (wp < pipes.size()) {
        dup2(pipes[wp][1], 1); // redirect stdout
        close(pipes[wp][1]);
    }
    if (rp > 0) {
        dup2(pipes[rp][0], 0); // redirect stdin
        close(pipes[rp][0]);
    }
}

void PipeHandler::pop(int p) {
    // std::cout << "parent close " << pipes[p][0] << ' ' << pipes[p][1] << '\n';
    close(pipes[p][0]);
    close(pipes[p][1]);
}

NumPipe::NumPipe(int _ttl) {
    ttl = _ttl;
    int _p[2];
    if (pipe(_p) < 0) {
        std::cerr << "can't create pipe\n";
        return;
    }
    pipeend.push_back(_p[0]);
    pipeend.push_back(_p[1]);
}

bool NumPipe::countdown(int _ttl) {
    if (_ttl == 0) {
        ttl -= 1;
    }
    return ttl == _ttl;
}

void NumPipe::shutdown() {
    close(pipeend[0]);
    close(pipeend[1]);
}

NumPipeHandler::NumPipeHandler() {
    npipes.clear();
}

int NumPipeHandler::size() {
    return npipes.size();
}

void NumPipeHandler::push(int ttl) {
    NumPipe npipe(ttl);
    if (!npipe.pipeend.empty()) {
        npipes.push_back(npipe);
    }
}

void NumPipeHandler::handle(std::vector<int> &nps, bool first, int numPipeType, int cnp) {
    int front, i = 0, npipessize;
    front = (nps.empty()) ? -1 : nps[i];
    // npipessize = (newNumPipe) ? npipes.size() - 1 : npipes.size(); // ignore the last npipes element if this is the numpipe command

    // std::cout << "nps \n";
    // for (int i = 0; i < nps.size(); i++) {
    //     std::cout << nps[i] << ' ';
    // }
    // std::cout << '\n';
    // std::cout << "front " << front << '\n';
    // std::cout << "npipessize " << npipessize << '\n';
    // std::cout << "first " << first << '\n';
    // std::cout << "numPipeType " << numPipeType << '\n';
    // std::cout << "wnp " << cnp << '\n'; 

    for (int p = 0; p < npipes.size(); p++) {
        if (p == cnp) { // if npipes[p] is the current create pipe 
            continue;
        }
        if (p != front || !first) { // close both end if npipes[p] is not the input pipe
            // std::cout << "close " << npipes[p].pipeend[0] << ' ' << npipes[p].pipeend[1] << '\n';
            npipes[p].shutdown();
        } else { // close write end if npipes[p] is the input pipe
            // std::cout << "close write end " << npipes[p].pipeend[1] << '\n';
            close(npipes[p].pipeend[1]);
            i += 1;
            front = nps[i];
        }
    }
    if (first && !nps.empty()) { // if this is the first command with numpipe, redirect stdin
        // how to deal with multiple input pipe?
        dup2(npipes[nps[0]].pipeend[0], 0);
        close(npipes[nps[0]].pipeend[0]);
    }
    if (numPipeType != 0) { // if this is the last command with numpipe, redirect stdout
        // int ep = npipes.size() - 1;
        // std::cout << "close read end " << npipes[cnp].pipeend[0] << '\n';
        close(npipes[cnp].pipeend[0]);

        if (numPipeType == 2) {
            // std::cout << "check dup2 error\n";
            dup2(npipes[cnp].pipeend[1], 2);
        }
        // std::cout << "check dup2 out\n";
        dup2(npipes[cnp].pipeend[1], 1);
        close(npipes[cnp].pipeend[1]);
    }
}

void NumPipeHandler::getTimeoutN(std::vector<int> &nps, int ttl) {
    int npipeid = -1;
    for (int np = 0; np < npipes.size(); np++) {
        if (npipes[np].countdown(ttl)) {
            nps.push_back(np);
        }
    }
}

void NumPipeHandler::pop(std::vector<int> &nps) {
    for (int i = 0; i < nps.size(); i++) {
        npipes[nps[i]].shutdown();
        // std::cout << "parent close num " << npipes[nps[i]].pipeend[0] << ' ' << npipes[nps[i]].pipeend[1] << '\n';
    }

    int offset = 0;
    for (int i = 0; i < nps.size(); i++) {
        npipes.erase(npipes.begin() + nps[i] - offset);
        offset++;
    }
    nps.clear();
}
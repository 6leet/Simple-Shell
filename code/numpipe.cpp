#include <iostream>
#include <vector>
#include <unistd.h>
#include "numpipe.h"


NumPipe::NumPipe(int _ttl) {
    ttl = _ttl;
    int _p[2];
    if (pipe(_p) < 0) {
        std::cerr << "can't create pipe\n";
    }
    pipeend.push_back(_p[0]);
    pipeend.push_back(_p[1]);
}
bool NumPipe::countdown() {
    ttl -= 1;
    return ttl == 0;
}
void NumPipe::shutdown() {
    close(pipeend[0]);
    close(pipeend[1]);
}

NumPipeHandler::NumPipeHandler() {
    npipes.clear();
}
void NumPipeHandler::push(int ttl) {
    NumPipe npipe(ttl);
    npipes.push_back(npipe);
}
void NumPipeHandler::handle(std::vector<int> &nps, bool first, bool isNumPipe) {
    int front, i = 0, npipessize;
    front = (nps.empty()) ? -1 : nps[i];
    npipessize = (isNumPipe) ? npipes.size() - 1 : npipes.size(); // ignore the last npipes element if this is the numpipe command

    for (int p = 0; p < npipessize; p++) {
        if (p != front || !first) { // close both end if npipes[p] is not the input pipe
            // cout << "close " << npipes[p].pipeend[0] << ' ' << npipes[p].pipeend[1] << '\n';
            npipes[p].shutdown();
        } else { // close write end if npipes[p] is the input pipe
            // cout << "close write end " << npipes[p].pipeend[1] << '\n';
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
    if (isNumPipe) { // if this is the last command with numpipe, redirect stdout
        int ep = npipes.size() - 1;
        // cout << "close read end " << npipes[ep].pipeend[0] << '\n';
        close(npipes[ep].pipeend[0]);
        dup2(npipes[ep].pipeend[1], 1);
        close(npipes[ep].pipeend[1]);
    }
}
void NumPipeHandler::getTimeout(std::vector<int> &nps) {
    int npipeid = -1;
    for (int np = 0; np < npipes.size(); np++) {
        if (npipes[np].countdown()) {
            nps.push_back(np);
        }
    }
}
void NumPipeHandler::pop(std::vector<int> &nps) {
    for (int i = 0; i < nps.size(); i++) {
        npipes[nps[i]].shutdown();
        // cout << "parent close num " << npipes[nps[i]].pipeend[0] << ' ' << npipes[nps[i]].pipeend[1] << '\n';
    }

    int offset = 0;
    for (int i = 0; i < nps.size(); i++) {
        npipes.erase(npipes.begin() + nps[i] - offset);
        offset++;
    }
    nps.clear();
}
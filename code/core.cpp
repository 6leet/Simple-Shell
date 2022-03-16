#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <tuple>
#include "handler.h"
// #include "numpipe.h"
#include "pipe.h"

using namespace std;

// TODO: 
// 1. `cat | ls`, what does cat do exactly?
// 2. test case 5 is correct when no init() was done, but wrong with init()
// 3. `ls |`
// 4. numbered pipe with large data
// 5. retry fork beautify

// void writeMsg() // deal with all outputs??? (maybe distribute with std & err)

NumPipeHandler npipeh;
int processCnt = 0;
int processLimit;

bool isPipe(string s) {
    return s == "|";
}

bool isBuiltin(string s) {
    return s == "exit" || s == "printenv" || s == "setenv";
}

// input, pre-process
// cmd[0] (read: empty,      write: pipe[1][1]) | _emptypipe_
// cmd[1] (read: pipe[1][0], write: pipe[2][1]) | pipe[1]
// cmd[2] (read: pipe[2][0], write: empty     ) | pipe[2]
void inputParser(vector<vector<string> > &cmds) {
    string _cmd, buf;
    getline(cin, _cmd);
    if (cin.eof()) {
        exit(0);
    }
    stringstream ss(_cmd);
    vector<string> cmd;
    while (ss >> buf) {
        if (isPipe(buf)) {
            if (!cmd.empty()) {
                cmds.push_back(cmd);
                cmd.clear();
            }
            continue;
        }
        cmd.push_back(buf);
    }
    if (!cmd.empty()) {
        cmds.push_back(cmd);
        cmd.clear();
    }
}

// type, np (id)
pair<int, int> checkNumPipe(vector<string> &cmd) {
    int end = cmd.size() - 1;
    if (cmd[end][0] == '|' || cmd[end][0] == '!') {
        int ttl;
        try {
            ttl = stoi(cmd[end].substr(1, cmd[end].size() - 1));
        } catch (invalid_argument) {
            return make_pair(0, -1);
        }
        int np;
        vector<int> nps;
        npipeh.getTimeoutN(nps, ttl);
        if (nps.empty()) {
            npipeh.push(ttl);
            np = npipeh.size() - 1; // last one
        } else {
            np = nps[0];
        }
        cmd.erase(cmd.begin() + end);
        return (cmd[end][0] == '|') ? make_pair(1, np) : make_pair(2, np);
    }
    return make_pair(0, -1);
}

// redirect, filename
pair<bool, string> checkRedirect(vector<string> &cmd) {
    bool redirect = false;
    string filename = "";
    for (int i = 0; i < cmd.size(); i++) {
        if (cmd[i] == ">") {
            redirect = true;
        } else if (redirect) {
            filename = cmd[i];
            cmd.erase(cmd.begin() + i);
            cmd.erase(cmd.begin() + i - 1);
            return make_pair(redirect, filename);
        }
    }
    return make_pair(redirect, filename);
}

int middleware(vector<string> cmd) { // 0: normal return, 1: unknown command
    if (isBuiltin(cmd[0])) {
        return 0;
    } else {
        return binHandler(cmd);
    }
    return -1;
}

int builtin(vector<string> cmd, bool last) {
    if (cmd[0] == "exit" && last) {
        exit(0);
    } else if (cmd[0] == "setenv") {
        return setenvHandler(cmd[1], cmd[2]);
    } else if (cmd[0] == "printenv") {
        if (cmd.size() < 2) {
            return printenvAllHandler();
        } else {
            return printenvHandler(cmd[1]);
        }
    }
    return -1;
}

void sigHandler(int signo) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // cout << "child " << pid << " terminated\n";
        processCnt -= 1;
    }
    return;
}

int handler() {
    vector<vector<string> > cmds;
    PipeHandler pipeh;
    inputParser(cmds);
    if (cmds.size() == 0) {
        return 0;
    }

    int pushedCmdid;
    vector<int> nps;
    npipeh.getTimeoutN(nps, 0);

    signal(SIGCHLD, sigHandler);
    
    for (int cmdid = 0; cmdid < cmds.size(); cmdid++) {
        vector<string> cmd = cmds[cmdid];
        bool first = (cmdid == 0), last = (cmdid == cmds.size() - 1);
        int p = cmdid;

        int numPipeType, cnp;
        tie(numPipeType, cnp) = checkNumPipe(cmd);
        bool hasNumPipe = (numPipeType != 0);

        bool redirect;
        string filename;
        tie(redirect, filename) = checkRedirect(cmd);

        if (!last && pushedCmdid != cmdid) {
            bool pushed;
            pushed = pipeh.push();
            if (pushed) {
                pushedCmdid = cmdid;
            }
        }

        // debug
        // for (int j = 0; j < cmd.size(); j++) {
        //     cout << cmd[j] << ' ';
        // }
        // cout << '\n';

        pid_t pid = fork();
        if (pid < 0) {
            // cerr << cmdid << " fork error\n";
            processLimit = processCnt;
            while (processCnt >= processLimit) {}
            cmdid -= 1;
            continue;
            // wait for signal (signal: child process killed)
            // cmdid -= 1; // retry fork()
            // continue;
        } else if (pid == 0) {
            pipeh.handle(p);
            npipeh.handle(nps, first, numPipeType, cnp);
            redirectHandler(redirect, filename);
            int res = middleware(cmd);
            if (res == -1) {
                cerr << "Unknown command: [" << cmd[0] << "].\n";
            }
            exit(0);
        } else {
            processCnt += 1;
            pipeh.pop(p);
            npipeh.pop(nps);
            if (last && !hasNumPipe) {
                int status;
                waitpid(pid, &status, 0);
                // cout << "parent catched " << cmdid << '\n';
            }
            builtin(cmd, last);
        }
    }
    return -1;
}

void init() {
    setenvHandler("PATH", "bin:.");
}

int main() {
    init();
    while (true) {
        cout << "% ";
        int res = handler();
    }
}

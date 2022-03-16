#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>


extern char **environ;

int printenvHandler(std::string var) {
    char* env = getenv(var.c_str());
    if (env != NULL) {
        std::cout << env << '\n';
    }
    return 0;
}

int printenvAllHandler() {
    char* env = *environ;
    for (int i = 1; env; i++) {
        std::cout << env << '\n';
        env = *(environ + i);
    }
    return 0;
}

int setenvHandler(std::string var, std::string value) {
    return setenv(var.c_str(), value.c_str(), 1);
}

int binHandler(std::vector<std::string> cmd) {
    std::vector<char*> argv;
    for (int i = 0; i < cmd.size(); i++) {
        argv.push_back(const_cast<char*>(cmd[i].c_str()));
    }
    argv.push_back(nullptr);
    // std::cout << "do " << cmd[0] << '\n';
    execvp(cmd[0].c_str(), argv.data());
    if (errno == 2) {
        return -1;
    }
    return 0;
}

void writeToFile(int readfd, std::string filename) {
    int writefd;
    writefd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    if (writefd == -1) {
        perror("open error");
        exit(EXIT_FAILURE);
    }
    char buf;
    while ((read(readfd, &buf, 1)) > 0) {
        write(writefd, &buf, 1);
    }
    chmod(filename.c_str(), 0777);
    close(readfd);
    close(writefd);
}

void redirectHandler(bool redirect, std::string filename) {
    if (!redirect) {
        return;
    }

    int rpipe[2];
    if (pipe(rpipe) < 0) {
        std::cerr << "can't create pipe\n";
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(rpipe[1]);
        writeToFile(rpipe[0], filename);
        exit(0);
    } else {
        close(rpipe[0]);
        dup2(rpipe[1], 1);
        close(rpipe[1]);
        return;
    }
}
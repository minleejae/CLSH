#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


//원격 명령어 파싱하는 함수
void getRemoteCommend(char parsing[][100], int start, int inputWords ,char remoteCommand[]){
    int idx = 0;
    for (int i = start; i < inputWords; i++) {
        for (int j = 0; parsing[i][j] != '\0'; j++) {
            remoteCommand[idx++] = parsing[i][j];
        }
        remoteCommand[idx++] = ' ';
    }
    remoteCommand[idx - 1] = '\0';
}


int parseInput(char buf[], char parsing[][100], const char *delimeter) {
    //문자열 파싱
    int words = 0;
    char *res = strtok(buf, delimeter);
    while (res != NULL) {
        strcpy(parsing[words++], res);
        res = strtok(NULL, delimeter);
    }

    return words;
}


int input() {
    char buf[128];
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf) - 1, stdin);
    buf[strlen(buf) - 1] = '\0';

    if (!strncmp(buf, "exit", strlen(buf))) {
        return -1;
    }

    //문자열 파싱
    char parsing[10][100] = {0};
    int inputWords = parseInput(buf, parsing, " ");


    char remoteCommand[200] = {0};


    //clsh 명령어
    if (!strncmp(parsing[0], "clsh", strlen(parsing[0]))) {
        printf("clsh 명령어 수행!");

        //clsh -h node1,node2,node3,node4 cat /proc/loadavg
        if (!strncmp(parsing[1], "-h", strlen(parsing[0]))) {
            char nodes[10][100] = {0};
            int nodesNum = parseInput(parsing[2], nodes, " ");

            printf("nodes!\n");
            for (int i = 0; i < nodesNum; i++) {
                printf("%s\n", nodes[i]);
            }


            getRemoteCommend(parsing, 3, inputWords, remoteCommand);

            printf("%s\n", remoteCommand);

        } else if (!strncmp(parsing[1], "--h", strlen(parsing[0]))) {

        }

    }
    return 0;
}

int main() {
    pid_t pid;
    while (1) {
        if (input() == -1) {
            return 0;
        }


//        pid = fork();
//
//        if (pid < 0) {
//            perror("fork error\n");
//            return -1;
//        } else if (pid == 0) {
//            char buf[128] = {0};
//            execlp(buf, buf, NULL);
//            printf("Child Process My PID:%d, My Parent's PID:%d, MY Childs'Pid %d\n", (int) getpid(), (int) getppid(),
//                   (int) pid);
//            exit(0);
//        } else {
//            printf("Parent Process My PID:%d, My Parent's PID:%d, MY Childs'Pid %d\n", (int) getpid(), (int) getppid(),
//                   (int) pid);
//            wait(NULL);
//        }

    }

    return 0;
}
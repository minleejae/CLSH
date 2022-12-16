#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


char *totalNodes[] = {"node1", "node2", "node3", "node4"};

//원격 명령어 파싱하는 함수
void getRemoteCommend(char parsing[][100], int start, int inputWords, char remoteCommand[]) {
    int idx = 0;
    for (int i = start; i < inputWords; i++) {
        for (int j = 0; parsing[i][j] != '\0'; j++) {
            remoteCommand[idx++] = parsing[i][j];
        }
        remoteCommand[idx++] = ' ';
    }
    remoteCommand[idx - 1] = '\0';
}


//문자열 파싱
int parseInput(char buf[], char parsing[][100], const char *delimeter) {
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

    //clsh 명령어
    if (!strncmp(parsing[0], "clsh", strlen(parsing[0]))) {
        char nodes[10][100] = {0};
        char remoteCommand[200] = {0};

        //clsh -h node1,node2,node3,node4 cat /proc/loadavg
        if (!strncmp(parsing[1], "-h", strlen(parsing[0]))) {
            int nodesNum = parseInput(parsing[2], nodes, ",");

            printf("nodes!\n");
            for (int i = 0; i < nodesNum; i++) {
                printf("%s\n", nodes[i]);

                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork error\n");
                    return -1;
                } else if (pid == 0) {
                    printf("Child Process My PID:%d, My Parent's PID:%d, MY Childs'Pid %d\n", (int) getpid(),
                           (int) getppid(),
                           (int) pid);
//                    char *exec_arg[] = {"ssh", nodes[i], "-l", "ubuntu"};
//                    execv("/bin/ssh", exec_arg);
                    exit(0);
                } else {
                    printf("Parent Process My PID:%d, My Parent's PID:%d, MY Childs'Pid %d\n", (int) getpid(),
                           (int) getppid(),
                           (int) pid);

                    char *exec_arg[] = {"ssh", nodes[i], "-l", "ubuntu"};
                    execv("/bin/ssh", exec_arg);
                    wait(NULL);
                }


            }

            getRemoteCommend(parsing, 3, inputWords, remoteCommand);
            printf("%s\n", remoteCommand);

        } else if (!strncmp(parsing[1], "--h", strlen(parsing[0]))) {

        }

    }
    return 0;
}

int main() {
    pid_t childPid[4] = {0};
    //ssh fork, exec
    int N = 1;
    pid_t pid[N]; /* process id */
    
    char buf[100];

    int childStatus;
    int i;

    for (i = 0; i < N; i++) {
        int fd1[2], fd2[2];

        //파이프 생성
        if (pipe(fd1) == -1) {
            perror("pipe");
            exit(1);
        }
        if (pipe(fd2) == -1) {
            perror("pipe");
            exit(1);
        }

        pid[i] = fork();
        if (pid[i] == 0) {
            close(fd1[1]);
            close(fd2[0]);

            //표준 입력을 fd1[0]이 가리키는 파이프에서 읽는다.
            dup2(fd1[0], 0);
            close(fd1[0]);

            //표준 출력을 파이프를 통해 출력
            dup2(fd2[1],1);
            close(fd1[1]);

            int len = read(0, buf, 6);
            write(1, buf, len);

//
//            char *exec_arg[] = {"ssh", totalNodes[i], "-l", "ubuntu", NULL};
//            execv("/bin/ssh", exec_arg);

            write(1, "Good\n", 6);
            printf("Now pid[%d] is die\n", i);
//            exit(100 + i);
        } else {
            close(fd1[0]);
            close(fd2[1]);

//            dup2(fd1[1],1);
//            close(fd1[1]);

//            dup2(fd2[0], 0);
//
            memset(buf, 0, sizeof(buf));
            fgets(buf, sizeof(buf) - 1, stdin);
            buf[strlen(buf) - 1] = '\0';

            write(fd1[1], buf, 20);
            int len = read(fd2[0], buf, 256);
            write(1, buf, len);

            wait(NULL);
        }
        // Create multiple child processes
    }

    for (i = 0; i < N; i++) {
        pid_t terminatedChild = wait(&childStatus);

        if (WIFEXITED(childStatus)) {
            // The child process has termindated normally

            printf("Child %d has terminated with exit status %d\n", terminatedChild, WEXITSTATUS(childStatus));
        } else
            printf("Child %d has terminated abnormally\n", terminatedChild);
    }


//    for (int i = 0; i < 4; i++) {
//        //ssh연결을 할 자식 프로세스
//        childPid[i] = fork();
//        if (childPid[i] == 0) {
//            printf("%s Child Process My PID:%d, My Parent's PID:%d, MY Childs'Pid %d\n", totalNodes[i], (int) getpid(),
//                   (int) getppid(),
//                   (int) childPid[i]);
//            char *exec_arg[] = {"ssh", totalNodes[i], "-l", "ubuntu"};
//            execv("/bin/ssh", exec_arg);
//            wait(NULL);
//        }
//    }
//
//    if (childPid[0] !=0){
//        printf("I'm Parent Process My PID:%d, My Parent's PID:%d, MY Childs'Pid %d\n", (int) getpid(),
//               (int) getppid(),
//               (int) childPid[0]);
//
//        for(int i=0; i<4; i++){
//            printf("%d\n",childPid[i]);
//        }
//    }

//    printf("----- ssh connected!------\n");
//
//    while (1) {
//        if (input() == -1) {
//            return 0;
//        }
//
//
//    }

    return 0;
}
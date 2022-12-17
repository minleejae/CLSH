#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define MSGSIZE 300


int connectedNodes = 0;
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
    int N = 4;
    pid_t pid[N]; /* process id */

    char buf[MSGSIZE];
    int childStatus;
    int i;

    int fd1[N][2], fd2[N][2];

    //ssh connect
    for (i = 0; i < N; i++) {
        //파이프 생성
        if (pipe(fd1[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        if (pipe(fd2[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        //read non block
        if (fcntl(fd2[i][0], F_SETFL, O_NONBLOCK) == -1) {
            perror("fcntl call");
        }

        pid[i] = fork();
        if (pid[i] == 0) { // Child Process
            close(fd1[i][1]);
            close(fd2[i][0]);

            //표준 입력을 fd1[0]이 가리키는 파이프에서 읽는다.
            dup2(fd1[i][0], STDIN_FILENO);
            close(fd1[i][0]);

            //표준 출력을 파이프를 통해 출력
            dup2(fd2[i][1], STDOUT_FILENO);
            close(fd1[i][1]);

            //buffer setting
            setvbuf(stdin, NULL, _IOLBF, 0);
            setvbuf(stdout, NULL, _IOLBF, 0);

            //ssh connect  명령어 :  sshpass -p ubuntu ssh -tt node1 -l ubuntu : 비밀번호 추가 입력없이 연결
            char *exec_arg[] = { "sshpass", "-p", "ubuntu",  "ssh", "-tt", totalNodes[i], "-l", "ubuntu", NULL};
            execv("/bin/sshpass", exec_arg);
        } else {
            close(fd1[i][0]);
            close(fd2[i][1]);

            //buffer setting
            setvbuf(stdin, NULL, _IOLBF, 0);
            setvbuf(stdout, NULL, _IOLBF, 0);

            write(fd1[i][1], "cat /proc/loadavg\n", 19);
        }
        // Create multiple child processes
    }


    int nread;
    while (1) {
//        input();

        for (int j = 0; j < N; j++) {
            switch (nread = read(fd2[j][0], buf, MSGSIZE)) {
                case -1:
                    if (errno == EAGAIN) {
                        sleep(1);
                        break;
                    } else perror("read call");
                case 0:
                    printf("End of conversation\n");
                    exit(0);
                default:
                    printf("MSG=%s\n", buf);
            }
        }
    }


    //wait
    for (i = 0; i < N; i++) {
        pid_t terminatedChild = wait(&childStatus);
        if (WIFEXITED(childStatus)) {
            printf("Child %d has terminated with exit status %d\n", terminatedChild, WEXITSTATUS(childStatus));
        } else
            printf("Child %d has terminated abnormally\n", terminatedChild);
    }


    return 0;
}
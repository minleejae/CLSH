#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define TOTAL_NODES_NUM 4
#define MSGSIZE 1000

char *totalNodes[] = {"node1", "node2", "node3", "node4"};
pid_t pid[TOTAL_NODES_NUM]; /* process id */


//원격 명령어 파싱하는 함수, 문자 개수 리턴
int getRemoteCommend(char parsing[][100], int start, int inputWords, char remoteCommand[]) {
    int idx = 0;
    for (int i = start; i < inputWords; i++) {
        for (int j = 0; parsing[i][j] != '\0'; j++) {
            remoteCommand[idx++] = parsing[i][j];
        }
        remoteCommand[idx++] = ' ';
    }
    remoteCommand[idx++] = '\n';
    remoteCommand[idx++] = '\0';
    return idx;
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

//모든 자식프로세스를 종료하는 함수
void terminateAllChildProcess(int sig) {
    int childStatus;
    //모든 자식프로세스에 sigstop 시그널 보내기
    for (int i = 0; i < TOTAL_NODES_NUM; i++) {
        kill(pid[i], sig);
    }

    //자식 프로세스가 끝날 때까지 wait
    for (int i = 0; i < TOTAL_NODES_NUM; i++) {
        pid_t terminatedChild = wait(&childStatus);
        if (terminatedChild <= 0) continue;
        printf("자식 프로세스(%d)가 종료되었습니다.\n", terminatedChild);
    }
    printf("모든 프로세스가 종료되었습니다.\n");
}


void sigTermHandler(int signo) {
    psignal(signo, "Received Signal:");
    terminateAllChildProcess(SIGTERM);
    exit(0);
}

void sigQuitHandler(int signo) {
    psignal(signo, "Received Signal:");
    terminateAllChildProcess(SIGKILL);
    exit(0);
}

void sigChildHandler(int signo) {
    pid_t pid_child;
    int node = -1;
    int status;
    while (1) {
        if ((pid_child = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("자식 프로세스(%d)가 종료되었습니다.\n", pid_child);
            for (int i = 0; i < TOTAL_NODES_NUM; i++) {
                if ((int)pid_child == (int)pid[i]) {
                    node = i;
                }
            }
        } else {
            break;
        }
    }
    printf("ERROR: %s connection lost\n", totalNodes[node]);
    terminateAllChildProcess(SIGTERM);
    exit(0);
}


int main() {
    char buf[MSGSIZE];
    int i;


    int fd1[TOTAL_NODES_NUM][2], fd2[TOTAL_NODES_NUM][2], fd3[TOTAL_NODES_NUM][2];

    //ssh connect
    for (i = 0; i < TOTAL_NODES_NUM; i++) {
        //파이프 생성
        if (pipe(fd1[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        if (pipe(fd2[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        if (pipe(fd3[i]) == -1) {
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
            close(fd2[i][1]);

            //stderr를 파이프를 통해 출력
            dup2(fd3[i][1], STDERR_FILENO);
            close(fd3[i][1]);

            //buffer setting
            setvbuf(stdin, NULL, _IOLBF, 0);
            setvbuf(stdout, NULL, _IOLBF, 0);

            //ssh connect  명령어 :  sshpass -p ubuntu ssh -tt node1 -l ubuntu : 비밀번호 추가 입력없이 연결
            char *exec_arg[] = {"sshpass", "-p", "ubuntu", "ssh", "-tt", totalNodes[i], "-l", "ubuntu", NULL};
            execv("/bin/sshpass", exec_arg);
        } else {
            close(fd1[i][0]);
            close(fd2[i][1]);
            close(fd3[i][1]);

            //buffer setting
            setvbuf(stdin, NULL, _IOLBF, 0);
            setvbuf(stdout, NULL, _IOLBF, 0);
        }
    }
    //child는 exec하기 때문에 아래 코드는 부모프로세스에서만 동작

    //SIGTERM, SIGINT, SIGTSTP, SIGQUIT 시그널 처리
    struct sigaction act, chld;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sigTermHandler;
    if (sigaction(SIGTERM, &act, (struct sigaction *) NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGINT, &act, (struct sigaction *) NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGTSTP, &act, (struct sigaction *) NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    act.sa_handler = sigQuitHandler;
    if (sigaction(SIGQUIT, &act, (struct sigaction *) NULL) < 0) {
        perror("sigaction");
        exit(1);
    }

    //sigchld 처리
    chld.sa_handler = sigChildHandler;
    sigfillset(&chld.sa_mask);
    chld.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &chld, NULL);


    //ssh connected 출력
    int nread;
    bool checkSshConnected[4] = {0};
    printf("모든 노드에 ssh 연결 중입니다.\n");
    while (1) {
        for (int j = 0; j < TOTAL_NODES_NUM; j++) {
            //이미 출력했으면 출력하지 않음
            if (checkSshConnected[j] == 1) continue;
            memset(buf, 0, sizeof(buf));
            switch (nread = read(fd2[j][0], buf, MSGSIZE)) {
                case -1:
                    if (errno == EAGAIN) {
                        sleep(1);
                        break;
                    } else perror("read call");
                case 0:
                    printf("End of conversation\n");
                    break;
                default:
                    checkSshConnected[j] = 1;
                    printf("%s:%s\n", totalNodes[j], buf);
            }
        }
        //모든 노드가 연결되었는지 여부 검사후 while문 종료
        bool flag = true;
        for (int i = 0; i < TOTAL_NODES_NUM; i++) {
            if (!checkSshConnected[i]) {
                flag = false;
            }
        }
        if (flag) {
            printf("모든 노드에 연결되었습니다.\n");
            break;
        }
    }


    //명령어 입력
    while (1) {
        //input
        char inputBuf[MSGSIZE] = {0};
        fgets(inputBuf, sizeof(inputBuf) - 1, stdin);
        inputBuf[strlen(inputBuf) - 1] = '\0';

        //명령을 전달한 노드가 출력을 완료했는지 여부 저장
        bool completedNodes[TOTAL_NODES_NUM];
        for (int i = 0; i < TOTAL_NODES_NUM; i++) {
            completedNodes[i] = 1;
        }

        //문자열 파싱
        char parsing[10][100] = {0};
        int inputWords = parseInput(inputBuf, parsing, " ");

        //명령을 보낼 노드와 명령어
        char nodes[10][100] = {0};
        char remoteCommand[200] = {0};
        int inputNodesNum = -1;
        int commandLength = -1;

        if (strlen(inputBuf) != 0 && !strncmp(inputBuf, "quit", strlen(inputBuf))) { //메인 프로세스 종료
            terminateAllChildProcess(SIGTERM);
            printf("모든 프로세스가 종료되었습니다.\n");
            return 0;
        } else if (!strncmp(parsing[0], "clsh", 4)) { //clsh 명령어
            //clsh -h node1,node2,node3,node4 cat /proc/loadavg
            if (!strncmp(parsing[1], "-h", 2)) {
                //명령을 보낼 노드 구하기
                inputNodesNum = parseInput(parsing[2], nodes, ",");
                commandLength = getRemoteCommend(parsing, 3, inputWords, remoteCommand);

            } else if ((!strncmp(parsing[1], "--hostfile", 10))) { //clsh --hostfile ./hostfile cat /proc/loadavg
                commandLength = getRemoteCommend(parsing, 3, inputWords, remoteCommand);
                int hostFileFd = -1;
                char hostFileBuf[MSGSIZE] = {0};
                //hostfile에서 읽기
                if ((hostFileFd = open("./hostfile", O_RDONLY)) > 0) {
                    int len = read(hostFileFd, hostFileBuf, MSGSIZE);
                    //파일에 적혀있는 노드 이름 파싱
                    inputNodesNum = parseInput(hostFileBuf, nodes, "\n");
                    close(hostFileFd);
                } else {
                    printf("%s 파일 열기에 실패했습니다.\n", parsing[2]);
                }
            } else {//--hostfile 옵션 생략, env에서 읽어오기
                commandLength = getRemoteCommend(parsing, 1, inputWords, remoteCommand);
                char *clsh_host_env = getenv("CLSH_HOSTS");
                char *clsh_hostfile_env = getenv("CLSH_HOSTFILE");
                int hostFileFd = -1;
                char hostFileBuf[MSGSIZE] = {0};

                if (clsh_host_env != NULL) { //CLSH_HOSTS 환경 변수에서 호스트 이름 읽어오기
                    printf("Note: use CLSH_HOSTS environment\n");
                    inputNodesNum = parseInput(clsh_host_env, nodes, ":");
                } else if (clsh_hostfile_env != NULL) { //hostfile에서 읽기
                    if ((hostFileFd = open(clsh_hostfile_env, O_RDONLY)) > 0) {
                        printf("Note: use hostfile '%s' (CLSH_HOSTFILE env)\n", clsh_hostfile_env);
                        int len = read(hostFileFd, hostFileBuf, MSGSIZE);
                        //파일에 적혀있는 노드 이름 파싱
                        inputNodesNum = parseInput(hostFileBuf, nodes, ":");
                        close(hostFileFd);
                    } else {
                        printf("%s 파일 열기에 실패했습니다.\n", clsh_hostfile_env);
                    }
                } else if ((hostFileFd = open(".hostfile", O_RDONLY)) > 0) { //현재 디렉토리에서 .hostfile에서 읽어오기
                    printf("Note: use hostfile '.hostfile' (default)\n");
                    int len = read(hostFileFd, hostFileBuf, MSGSIZE);
                    //파일에 적혀있는 노드 이름 파싱
                    inputNodesNum = parseInput(hostFileBuf, nodes, ":");
                    close(hostFileFd);
                } else {
                    printf("--hostfile 옵션이 제공되지 않았습니다\n");
                }
            }

            //interactive mode 확인
            if (!strncmp(remoteCommand, "-i", 2)) {
                printf("Enter 'quit' to leave this interactive mode\n");
                printf("Working with nodes: ");
                //노드 이름 출력
                for (int i = 0; i < inputNodesNum; i++) {
                    printf("%s", nodes[i]);
                    if (i == inputNodesNum - 1) {
                        printf("\n");
                    } else {
                        printf(", ");
                    }
                }
                //input 받기
                while (1) {
                    //input
                    bool interactiveNodes[TOTAL_NODES_NUM];
                    memcpy(interactiveNodes, completedNodes, sizeof(interactiveNodes));

                    printf("clsh>");
                    //명령 입력받기
                    char interactiveBuf[MSGSIZE] = {0};
                    fgets(interactiveBuf, sizeof(interactiveBuf) - 1, stdin);
                    if (!strncmp(interactiveBuf, "quit", 4)) {
                        printf("interactive mode를 종료합니다.\n");
                        break;
                    }

                    //로컬에서 실행하는 경우
                    if (interactiveBuf[0] == '!') {
                        write(1, "LOCAL:", 6);
                        sleep(1);
                        for (int i = 0; i < MSGSIZE - 1; i++) {
                            interactiveBuf[i] = interactiveBuf[i + 1];
                        }
                        system(interactiveBuf);
                        continue;
                    }

                    //노드 이름 비교하고 명령 보내기
                    for (int i = 0; i < inputNodesNum; i++) {
                        //미리 선언한 totalNodes와 이름 비교하고 명령어 전달
                        for (int j = 0; j < TOTAL_NODES_NUM; j++) {
                            if (!strncmp(totalNodes[j], nodes[i], strlen(nodes[i]))) {
                                interactiveNodes[j] = 0;
                                write(fd1[j][1], interactiveBuf, strlen(interactiveBuf));
                            }
                        }
                    }

                    printf("--------------------------\n");
                    //파이프에 입력받은 내용 main에 출력
                    while (1) {
                        for (int j = 0; j < TOTAL_NODES_NUM; j++) {
                            char interactiveOutputBuf[MSGSIZE] = {0};
                            if (interactiveNodes[j] == 1) continue;
                            switch (nread = read(fd2[j][0], interactiveOutputBuf, MSGSIZE)) {
                                case -1:
                                    if (errno == EAGAIN) {
                                        sleep(1);
                                        break;
                                    } else perror("read call");
                                case 0:
                                    printf("End of conversation\n");
                                    break;
                                default:
                                    interactiveNodes[j] = 1;
                                    char res[10][100] = {0};
                                    //문자열 파싱
                                    parseInput(interactiveOutputBuf, res, "\r\n");
                                    printf("%s:%s\n", totalNodes[j], res[2]);
                            }
                        }
                        //요청이 모두 출력되었으면 while문 종료
                        bool flag = true;
                        for (int j = 0; j < TOTAL_NODES_NUM; j++) {
                            if (interactiveNodes[j] == 0) {
                                flag = false;
                                break;
                            }
                        }
                        if (flag) {
                            break;
                        }
                    }
                    printf("--------------------------\n");
                }

                continue;
            }

            //노드 이름 비교하고 명령 보내기
            for (int i = 0; i < inputNodesNum; i++) {
                //미리 선언한 totalNodes와 이름 비교하고 명령어 전달
                for (int j = 0; j < TOTAL_NODES_NUM; j++) {
                    if (!strncmp(totalNodes[j], nodes[i], strlen(nodes[i]))) {
                        completedNodes[j] = 0;
                        write(fd1[j][1], remoteCommand, commandLength);
                    }
                }
            }
        } else {
            printf("명령어를 다시 입력해주세요.\n");
            continue;
        }

        //파이프에 입력받은 내용 main에 출력
        while (1) {
            for (int j = 0; j < TOTAL_NODES_NUM; j++) {
                //이미 출력했으면 출력하지 않음
                memset(buf, 0, sizeof(buf));
                if (completedNodes[j] == 1) continue;
                switch (nread = read(fd2[j][0], buf, MSGSIZE)) {
                    case -1:
                        if (errno == EAGAIN) {
                            sleep(1);
                            break;
                        } else perror("read call");
                    case 0:
                        printf("End of conversation\n");
                        break;
                    default:
                        completedNodes[j] = 1;
                        char procLoadAvg[10][100] = {0};

                        //문자열 파싱
                        parseInput(buf, procLoadAvg, "\r\n");
                        printf("%s:%s\n", totalNodes[j], procLoadAvg[2]);
                }
            }

            //요청이 모두 출력되었으면 while문 종료
            bool flag = true;
            for (int j = 0; j < TOTAL_NODES_NUM; j++) {
                if (completedNodes[j] == 0) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                break;
            }
        }
    }

    return 0;
}
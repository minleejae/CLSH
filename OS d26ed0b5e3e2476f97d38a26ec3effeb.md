# OS

# 7-2

### **공유메모리 (System V)**

```c
//공유메모리 생성,  shmid 리턴
int shmget(key_t key, size_t size, int shmflg)

//공유메모리 연결 , shmflg 0:읽기,쓰기 가능, SHM_RDONLY(읽기 전용), shmaddr 리턴
void *shmat(int shmid, const void *shmaddr, int shmflg);

//공유메모리 연결 해제
int shmdt(char *shmaddr);

//공유메모리 제어
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```

공유메모리 제어

cmd : 수행할 제어 기능 (IPC_RMID, IPC_SET, IPC_STAT, SHM_LOCK, SHM_UNLOCK)

**다른 프로세스가 사용중인 공유 메모리를 삭제하면?**

리눅스 기준으로 삭제가 예약되고 마지막 프로세스가 사용 해제할 때 삭제 된다.

ipcs 명령에서 상태가 dest(destoryed)로 표시된다.

**삭제하지도 않고, 아무도 사용 중이지 않은 공유메모리는?**

마지막에 기록된 상태 그대로 보존된다. 계속 메모리를 점유하고, 시스템이 재부팅되면 사라진다.

**공유메모리 장점**

주소에 직접 접근해 함수 호출이 필요없다.

위치를 알고 있어 포인터를 이용해 접근하여 메모리에서 읽기와 쓰기를 직접 수행한다.

**공유메모리 단점**

여러 프로세스나 스레드가 공유 메모리를 사용하는 경우

배타적 접근이 필요하다면 직접 개발해야만 한다. → 락 필요

### **세마포어 (System V)**

프로세스간 공유하는 락

P연산 → 자원 가져가기, 가져갈 자원이 없으면 대기

V연산 → 자원 반납, 대기하고 있는 프로세스가 있으면 시그널 보내 깨우기

```c
//세마포어 생성
int semget(key_t key, int nsems, int semflg);

//세마포어 제어
int semctl(int semid, int semnum, int cmd, ...);

//세마포어 연산
int semop(int semid, struct sembuf *sops, size_t nsops);

```

buf.sem_op 값이 음수면 P연산(잠금)이고, 양수면 V연산(잠금 해제)이다. , 0인경우 0이 될때까지 기다린다는 뜻이다.

semval과 sem_op의 값에 따라 동작이 다르다.

**XSI 세마포어**

오래된 표준, 대부분의 시스템에 적용

상태 취소(undo) 기능이 탑재되어있다.

wait-for-zero 연산을 제공한다(P/V가 반대로 동작)

strong semaphore가 구현되는 경우가 많다

**POSIX 세마포어**

Weak Semaphore로 구현되는 경우가 많다.

# 8-1

공유메모리 활용 (Cache, Data Sharing)

공유메모리 기법을 사용한 소프트웨어(Redis, Memcached)

## CACHE

캐시에 접근하는 시간이 원 데이터에 접근 혹은 값을 계산하는 시간보다 빠른 경우 사용한다.

캐시 종류 : CPU 캐시(L1, L2, L3), 디스크 캐시

여러 개의 프로세스로 나누어 처리하는 경우 병렬처리 / 분산처리 기술이 필요하다.

Read Cache : 데이터 읽기 성능 개선

Write Cache : 쓰기 속도 극복, 프로세스는 빠른 캐시에 쓰고, 나중에 원 데이터에 반영한다.

# 9-1

POSIX 공유메모리, 세마포어, 메시지 큐

### POSIX 공유 메모리

POSIX와 XSI 공유메모리는 서로 구조가 달라, 한쪽이 다른 기능을 랩핑하기 어렵다. (리얼타임 라이브러리를 사용한다, 컴파일할 때 링크해주어야 함)

**POSIX 장점**

직관적이고 일관된 인터페이스를 갖는다. 파일 기술자를 사용하는 인터페이스 형태이다.

POSIX의 다른 표준과 호환성이 높다.

```c
//공유 메모리 생성, 기존의 유닉스 파일 처리 개념을 확장함, name은 항상 /로 시작한다. 메모리의 일부를 파티션 공간처럼 만들고, 여기에 파일을 만들어 공유하는 방식이다.
int shm_open(const char* name, int oflag, mode_t mode);

//공유 메모리의 삭제
int shm_unlink(const char *name);

```

**open vs shm_open**

open은 DISK 파일 시스템의 파일을 이용하여 공유하고,

shm_open은 메모리의 일부를 파일 시스템으로 표현한 /dev/shm 에서 파일 개념을 표현하고 이 파일을 이용하여 공유한다.

파일의 경로는 /dev/shm 을 /로 한 경로를 사용한다.

POSIX 공유메모리는 기존의 파일을 사용한 mmap 기법의 프로그램이다.

shm_open & mmap 으로 쉽게 변경이 가능하다.

제한사항으로는 디렉토리 없이 파일에 쓰기 때문에 파일의 경로에 디렉토리 구조는 허용하지 않는다.

**POSIX 공유메모리의 삭제**

파일 삭제하는 unlink와 동일하다.

mmap으로 맵핑 중이면, munmap을 먼저 사용하여 해제하기

**POSIX 공유메모리를 사용 중에 삭제하면?**

표준안에 명시되어 있지는 않다.

리눅스에서는 새로운 프로세스는 공유 메모리에 연결이 불가하고, 기존 프로세스들은 공유 상태를 유지한다.

### POSIX 세마포어

명명된 세마포어(named semaphore), 익명 세마포어(nameless/anonymous semaphore) → fifo와 pipe의 차이

외부에서 접근 가능한 인터페이스 경로 유무에 따라 구분한다.

POSIX 세마포어는 스레드 라이브러리를 사용한다.

```c
//익명 세마포어의 생성, sem에는 생성된 posix 세마포어를 저장하고, pshared가 0이면 현재 프로세스에서만 사용, zero가 아니면 여러 프로세스에서 공유 목적-> 이경우 명명된 세마포어 쓰는게 좋음
int sem_init(sem_t *sem, int pshared, unsigned int value);

//명명된 세마포어 생성, open함수와 유사하다, value는 초기화할 세마포어의 값, name은 항상 /로 시작
sem_t *sem_open(const char *name, int ofalg);
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);

//POSIX 세마포어 연산 : P, V
//P 연산 sem_wait, sem_trywait, sem_timedwait -> POSIX 세마포어는 한번에 -1씩만 가능하다. 리소스가 없으면 대기한다.
int sem_wait(sem_t *sem); // ->리소스가 없으면 대기
int sem_trywait(sem_t *sem); // -> 리소스가 없으면, 에러 반환하고 errno에 EAGAIN이 설정된다.
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout); // -> 리소스가 없으면 지정한 시간만큼 대기하고, 대기 이후에도 없으면 에러 반환하고, errno에 ETIMEOUT설정됨
//sem timedwait는 UNIX 시간 표현으로 절대시간이다. 10초 대기하려면 현재 시간에 + 10초 -> time(NULL) + 10;

//V 연산, 리소스를 +1, 대기 상태가 없다. 실패: 최댓값(SEM_VALUE_MAX)보다 커지는 경우 errno에 EOVERFLOW가 설정된다.
int sem_post(sem_t *sem);

//익명 세마포어 제거, 익명 세마포어는 프로세스가 종료되면 자연스럽게 해제된다, 직접 제거하고자 할 경우 사용한다.
int sem_destroy(sem_t *sem);

//명명된 세마포어의 제거 : sem_close, sem_unlink, sem_close를 통해 닫고, 생성된 세마포어 파일을 삭제하기 위해 sem_unlink를 사용한다.
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
```

**명명된 세마포어**

생성된 세마포어는 /dev/shm/ 의 경로에 생성된다.

POSIX 공유메모리 처럼 “/m.sem” 파일을 sem_open하면, /dev/shm/sem.m.sem 파일이 생성된다.

oflag → O_CREAT, O_EXCL이 사용 가능하다.

실패시 SEM_FAILED를 반환한다.

**공유메모리**와 **메시지 큐**는 **리얼타임 라이브러리**를 링크해야 사용가능하고,

**세마포어**는 **pthread**를 링크해야 사용이 가능하다.

# 9-2

시그널의 기본 개념

시그널 보내는 방법

시그널 받아서 처리하는 기본적인 방법

시그널 집합의 개념과 사용방법

시그널 관련 기타 함수들의 사용 방법

```c
//시그널 보내기 pid>0 : pid로 지정한 프로세스에 시그널 발송, pid < -1 : 프로세스 그룹 ID가 pid의 절댓값인 프로세스 그룹에 속하고 시그널을 보낼 권한을 갖고 있는 모든 프로세스에 시그널 발송
//pid == 0 프로세스 그룹 ID가 시그널을 보내는 프로세스의 그룹 ID와 같은 모든 프로세스에게 시그널 발송
//pid == -1 : 시그널을 보내는 프로세스의 유효 사용자ID가 root가 아니면, 특별한 프로세스를 제외하고 프로세스의 실제 사용자 ID가 시그널을 보내는 프로세스의 유효 사용자 ID와 같은 모든 프로세스에 발송
int kill(pid_t pid, int sig);

//함수를 호출한 프로세스에 시그널 발송
int raise(int sig);

//함수를 호출한 프로세스에 SIGABRT 시그널 발송 -> 종료시키고 코어 덤프
void abort(void);

//시그널 핸들러 지정 -> 한번 호출된 후 기본동작으로 설정
typedef void(*sighandler_t)(int);
sighandler_t signal(int sigunm, sighandler_t handler);

void (*hand)(int);
hand = signal(SIGINT, handler);

//시그널 핸들러 지정 -> 시그널 핸들러가 한번 호출된 후 자동으로 다시 재지정
typedef void(*sighandler_t)(int);
sighandler_t sigset(int sigunm, sighandler_t handler);

//시그널 집합 : 시그널을 개별적으로 처리하지 않고 복수의 시그널을 처리하기 위해 도입(POSIX)
//시그널 집합의 처리를 위한 구조체:sigset_t, 시그널을 비트 마스크(128bit)로 표현, 각 비트가 시그널과 1:1로 연결, 비트 값이 1이면 해당 시그널 설정, 0이면 시그널 설정이 안된 것
typedef struct{
	unsigned int __sigbits[4];
} sigset_t;

//시그널 집합 처리 함수
//시그널 집합 비우기 -> 시그널 집합에서 모든 시그널을 0으로 설정
int sigemptyset(sigset_t *set);
//시그널 집합에 모든 시그널 설정 -> 시그널 집합에서 모든 시그널을 1로 설정
int sigfillset(sigset_t *set);
//시그널 집합에 시그널 설정 추가
int sigaddset(sigset_t *set, int signo);
//시그널 집합에 시그널 설정 삭제
int sigdelset(sigset_t *set, int signo);
//signo로 지정한 시그널이 시그널 집합에 포함되어 있는지 확인
int sigismember(sigset_t *set, int signo);

//기타 시그널 처리 함수
//시그널 정보 출력 -> s에 지정한 문자열을 붙여 정보를 출력
void psignal(int sig, const char *s);
//시그널 정보 출력, 인자로 받은 시그널을 가리키는 이름을 문자열로 리턴
char *strsignal(int sig);
//시그널 블록킹과 해제 -> 시그널을 처리 안하고 보류, 후에 해제할 때 보류된 시그널이 몇개인지 구분을 못하는 문제가 있다.
int sighold(int sig);
int sigrelse(int sig);

//시그널 집합 블록과 해제
int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset);
//how: 시그널을 블록할 것인지, 해제할 것인지 여부 설정
//SIG_BLOCK : set에 지정한 시그널 집합을 시그널 마스크에 추가
//SIG_UNBLOCK : set에 지정한 시그널 집합을 시그널 마스크에서 제거
//SIG_SETMASK : set에 지정한 시그널 집합으로 현재 시그널 마스크 대체
//set : 블록하거나 해제할 시그널 집합 주소
//oset : NULL 또는 이전 설정값을 저장한 시그널 집합 주소

//시그널 대기 -> 시그널이 올 때까지 대기할 시그널, 구식 API임, sigsuspend사용
int sigpause(int sig); 
// 시그널 기다리기, set : 기다리려는 시그널을 지정한 시그널 집합, 시그널 집합에 포함된 시그널은 모두 블록된다.
int sigsuspend(const sigset_t *set);
// 시그널 기다리기 
int sigwait(const sigset_t *set, int *sig); // set: 기다리려는 시그널을 지정한 시그널 집합 -> 시그널 집합에 포함된 시그널만 기다린다, 다른 시그널은 블럭되지 않는다.
// 시그널 보내기 idtype: id에 지정한 값의 종류, id: 시그널을 받을 프로세스나, 그룹, sig: 보내려는 시그널
int sigsend(idtype_t idtype, id_t id, int sig);
// 시그널 무시처리, 인자로 지정한 시그널의 처리방법을 SIG_IGN으로 설정
int sigignore(int sig);

```

SIGNAL 함수 호환성

SVR4 시스템과 BSD 시스템 간의 동작이 다름

SVR4 → 시그널 중첩 문제를 해결하기 위해 1번 호출한 뒤, 기본 동작으로 전환

BSD → 영구적인 시그널 핸들러로 동작하도록 수정됨

사용하지 말것 : signal, sigset, sighold, sigrelse, sigignore, sigpause

앞으로 개발할 때 이식성을 고려한다면 sigaction 사용

프로세스가 fork될 때 자식프로세스는 부모 프로세스의 시그널 처리를 상속받는다.

프로그램이 exec 될때는 기본 동작이 정의된 모든 시그널은 기본 동작으로 변경된다.

기본 동작이 없는 시그널은 그대로 둔다.

백그라운드 프로세스는 인터럽트 및 종료 신호의 처리를 무시하도록 설정한다.

많은 대화형 프로그램은 백그라운드에 있지 않을 때만 신호를 포착한다.

# 10-1

**sigaction 함수를 사용해 시그널을 처리 , 알람시그널**

### **sigaction 함수를 사용해 시그널을 처리**

signal, sigset 함수보다 다양하게 시그널 제어가 가능하다.

```c
// sig : 처리할 시그널(SIGKILL과 SIGSTOP은 변경 불가능), act : 시그널을 처리할 방법을 지정한 구조체 주소, oact : 기존에 시그널을 처리하던 방법을 저장할 구조체 주소(백업용)
int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact);

// sigaction 구조체
struct sigaction {
	int sa_flags; // 시그널 전달 방법을 수정할 플래그, SA_SIGINFO가 설정되어 있으면 sa_sigaction 멤버를 사용한다, 아니면 sa_handler를 사용한다.
	union { // OS에 따라 공용체로 선언되어 있을 수 있고, 아닐 수도 있음
	void (*sa_handler)(int);
	void (*sa_sigaction)(int, siginfo_t *, void *); //시그널 번호, //추가정보(발생시킨 PID, 시간, 이벤트 등의 정보), // void* (인터럽트된 컨텍스트의 정보)
	} _funcptr;
	sigset_t sa_mask; //시그널 핸들러가 수행되는 동안 블록될 시그널을 지정한 시그널 집합
};

//시그널 발생 원인 출력 psiginfo(3), pinfo: 시그널 발생원인 정보를 저장한 구조체, s: 출력할 문자열
void psiginfo(siginfo_t *pinfo, char*s);
```

**SIGACTION 구조체**

**sa_flags**

**SA_RESETHAND**

이 값을 설정하고 시그널을 받으면 시그널의 기본 처리 방법은 SIG_DFL로 재설정되고,

시그널이 처리되는 동안 시그널을 블록하지 않는다.

**SA_RESTART**

일반 : 대부분의 시스템 호출함수는 시그널이 수신되면 EINTR로 반환하고 종료한다.

SA_RESTART를 설정하면 EINTR 반환 대신, 자동으로 시스템 호출 함수에 재진입한다.

단 poll과 같은 타임아웃 기능이 있는 시스템 콜 함수는 여전히 EINTR을 발생시킨다. 이는 리얼타임 처리에서 중요하다.

**SA_NODEFER( == SA_NOMASK)**

일반 : 같은 시그널이 연속으로 도착하는 경우 첫번째 시그널은 처리(인터럽트)하고 시그널이 처리되는 동안, 동일한 시그널은 무시된다.

SA_NODEFER설정을 하면 동일한 시그널에 대해서 중첩처리 된다. 시그널이 처리되는 동안 동일한 시그널이 수신되면,

현재 시그널 처리를 중단하고 새로운 시그널 처리를 시작한다. → 같은 시그널이 무한으로 발생하는 경우 스택 오버플로우가 발생하는 문제가 있다.

**SA_SIGINFO**

시그널 번호 뿐만 아니라 추가적인 정보가 필요하면 설정할 수 있다.

SA_SIGINFO를 설정하면 sa_handler(int) 대신 sa_sigaction(int, siginfo t*, void *)를 핸들러 함수로 사용한다.

첫번째 인수 int는 시그널 번호, 두번째 인수 siginfo_t*는 추가정보(발생시킨 PID, 시간, 이벤트 등의 정보), 세번째 인수 void* 는 인터럽트 된 컨텍스트의 정보이다.

일반적인 시그널 처리에서는 잘 사용하지 않고, 리얼타임 시그널 처리에서 주로 사용한다.

**SA_ONSTACK**

시그널 핸들러가 사용할 스택을 대체스택으로 사용한다. 대체 스택 공간을 signalstack으로 미리 지정해야 한다.

주로 디버깅을 목적으로 사용하고, SIGSEGV 시그널인 경우 작동한다.

**SA_NOCLDWAIT**

이 값이 설정되어 있고 시그널이 SIGCHLD면 시스템은 자식 프로세스가 종료될 때 좀비 프로세스를 만들지 않는다.

**SA_NOCLDSTOP**

이 값이 설정되어 있고 시그널이 SIGCHLD면 자식 프로세스가 중지 또는 재시작할 때 부모 프로세스에 SIGCHLD 시그널을 전달하지 않는다.

**sa_mask**

비트가 1이면 **블록킹**한다. 블록킹된 시그널은 처리가 보류된다. 현재 시그널 핸들러가 종료된 후 처리한다.

비트가 0이면 **중첩**된다. 시그널 핸들러가 실행되는 동안, 다른 시그널이 도착하면 현재 시그널 핸들러가 잠시 정지되고 새로운 시그널의 핸들러가 동작한다.

시그널 집합 함수인 sigemptyset, sigfillset, sigaddset, sigdelset, sigismember 함수로 조작한다.

**시그널 핸들러의 상속**

**fork**

부모 프로세스의 시그널 핸들러 설정이 자식 프로세스에 상속된다.

설치하기 전(미묘한 시간차이)에 시그널이 발생할 수도 있어서, 자식 프로세스에서 설치할 때 문제가 생긴다.

단, 펜딩(시그널이 발생하여 블록)된 시그널은 상속이 되지 않는다.

설정만 상속되고, 일어난 현상은 상속되지 않는다.

**시스템 호출의 인터럽트**

**느린 시스템 호출 → 호출시 블록됨**

read / write ←→ pipes, terminal, network

open something (pipe, device, …)

pause, wait 계열

ioctl(), IPC 호출 함수

MAN Page에서 EINTR에 대한 설명이 있는 함수들

**느린 시스템 호출 → 시그널에 의해 인터럽트 될 수 있다.**

인터럽트가 된 시스템 호출

error와 errno에 EINTR을 저장후 종료한다. 에러는 대부분의 경우 음수(-1)로 반환한다.

시그널 처리 후 정상 동작을 하려면, 느린 시스템 호출 사용시 EINTR에 대한 처리를 해야만 한다.

```c
Again:
if((n=read(fd, buff, BUFFSIZE)) <0){
	if(errno == EINTR) go to Again;
}
```

**빠른 시스템 호출**

시그널이  도착하기 전에 종료된다.

EINTR이 있는 시스템 호출은 모두 재시작이 가능할까?

소켓, 시그널, I/O 멀티플렉싱, 슬립과 같은 블럭킹 함수는 재진입되지 않는다.

POSIX.1 규격

시스템 호출 함수는 재진입을 보장하도록 명세한다.

재진입이 허용되지 않는 시스템 호출도 있다. malloc() 중에 인터럽트가 된다면 예측이 불가능하고 인터럽트 되지 않는다.

시스템 호출 재진입 사용시 주의

errno 변수는 전역 변수로 1개 밖에 없다. errno를 확인하고 처리하려는 동안 또 시그널에 의해 인터럽트가 될 수 있다.

시그널 처리시 저장 후 복원하는 등의 처리 기법을 활용한다.

### **알람시그널**

일정한 시간이 지난 후에 자동으로 시그널이 발생하도록 하는 시그널이다. 일정 시간 후에 한 번 발생시키거나, 일정 간격을 두고 주기적으로 발송이 가능하다.

```c
//알람 시그널 생성, sec: 알람을 발생시킬 때까지 남은 시간(초 단위), 일정 시간이 지나면 SIGALRM 시그널 발생, 프로세스 별로 알람시계가 하나 밖에 없으므로 알람은 하나만 설정 가능
unsigned int alarm(unsigned int sec);

// 타이머 정보 검색, which: 타이머의 종류, value: 타이머 정보 구조체, 새로 설정
int getitimer(int which, struct itimerval *value);

// 타이머 설정 which: 타이머의 종류, value: 타이머 정보 구조체, 새로 설정, ovalue: 타이머 정보 구조체, 이전 정보(백업)
int setitimer(int which, const struct itimerval *value, struct itimerval *ovalue);
```

**타이머의 종류 (which)**

ITIMER_REAL : 실제 시간 사용, SIGALRM 시그널 발생

ITIMER_VIRTUAL : 프로세스의 가상 시간 사용, SIGVTALRM 시그널 발생

ITIMER_PROF :  시스템이 프로세스를 위해 실행중인 시간과 프로세스의 가상 시간을 모두 사용, SIGPROF 시그널 발생

ITIMER_REALPROF : 실제 시간 사용, 멀티 스레드 프로그램의 실제 실행시간 측정시 사용, SIGPROF 시그널 발생

### SIGCHLD 시그널

**좀비 프로세스**

뒤처리가 안 되어서 점유한 자원이 풀어지지 않은 프로세스 상태

프로세스 종료 과정에서 이미지가 해제되었지만, 메타데이터의 일부분(PCB의 일부)이 해제가 오래 걸리거나 해제가 안되는 경우가 문제가 발생한다.

모든 프로세스는 아주 잠깐 좀비가 될 수 있다.

프로그래머의 실수로 오랫동안 자식프로세스의 상태를 정리하지 않으면, 운영체제가 정리하지 않아 이를 좀비 프로세스라 부른다.

ps 명령으로 확인이 가능하다. 상태가 defunct 혹은 Z로 표현된다.

좀비 프로세스를 방지하지 않으면 운영체제가 관리하는 프로세스 관리 자료구조에 영향을 주어 자원 부족현상이 생길 수 있다.

**PCB의 일부가 유지되는 이유**

시스템 차원에서 프로세스의 성공과 실패 여부를 판단해 계층적으로 보고한다.

부모 프로세스와 자식 프로세스의 관계에서 일반적으로 부모 프로세스는 자식 프로세스의 성공/실패 여부를 알고자 한다.

이러한 전달은 프로세스가 아닌 운영체제가 하는 일이다.

즉, 프로세스 종료 시 대부분은 해제되지만, 상태 보고를 위해 PCB의 일부를 남긴다.

**좀비 프로세스 방지**

기본 : 자식 프로세스의 상태를 읽으면 된다. wait, waitpid, waitid 등의 시스템 호출을 사용한다.

1. 시그널을 통해 자식프로세스 정리하기
2. 자식프로세스 상태에 관심이 없다면
    1. SIGCHLD 시그널을 무시하기
    2. sigaction 에서 SIGCHLD의 핸들러를 SIG_IGN 핸들러로 설치하기

SIGCHLD는 프로세스가 STOP 되거나 종료된 경우 모두 발생한다.

자식프로세스의 STOP은 관심이 없지만, 종료된 경우만 관심있을 경우 SIGCHLD의 시그널 핸들러에 SA_NOCLDSTOP을 지정하여 설치한다.

SA_NOCLDSTOP을 설정하면 SIGSTOP, SIGTSTP, SGTTIN, SIGTTOU 같은 정지 시그널로 자식 프로세스가 정지된 경우 SIGCHLD 시그널 핸들러가 동작하지 않는다.

**wait 함수**

```c
pid_t wait(int *stat_loc); // stat_loc은 프로세스의 상태 정보
pid_t waitpid(pid_t pid, int *stat_loc, int options);

// stat_loc 을 검사하는 매크로
// 정상종료인 경우
WIFEXITED(stat_loc) // 자식 프로세스가 종료한 경우 non-zero 반환(True)
WEXITSTATUS(stat_loc) // WIFEXITED(..) 가 참인 경우에만 사용, 자식 프로세스의 종료 값을 반환한다.
//시그널로 종료된 경우
WIFSIGNALED(stat_loc) // 자식 프로세스가 시그널로 종료된 경우 non-zero(참)
WTERMSIG(stat_loc) // WIFSIGNALED 가 참인 경우에만 사용, 자식 프로세스가 수신한 종료 시그널 번호를 반환
// 정지된 경우
WIFSTOPPED(stat_loc) // 자식 프로세스가 정지된 경우 non-zero(참)
WSTOPSIG(stat_loc) // WIFESTOPPED 가 참인 경우에만 사용, 자식 프로세스가 수신한 정지 시그널 번호를 반환
```

waitpid는 반복문으로 확인해서 처리하자,

좀비 프로세스는 넌블럭킹(WNOHANG)으로 처리하여 빠르게 검사하고 반복문을 빠져 나올 수 있게 한다.

확장된 시그널 처리를 위한 siginfo_t를 사용한다.

wait이나 waitpid에 비해 세밀한 처리가 가능하다.

```c
int waitid(idytype_t idtype, id_t id, siginfo_t *infop, int options);
```

idtype : waitpid에서 첫 번째 인수 처리 방법의 역할

P_PID : 두 번째 인수 id : PID, PID가 id인 자식 프로세스를 가리킨다.

P_PGID : 두번 번째 인수 id : PGID, 프로세스 그룹 ID에 속한 자식 프로세스들을 가리킨다.

P_ALL : 자식 프로세스 중 아무나, 두번 째 인수 id가 무시된다.

**infop: 자식 프로세스의 종료 정보, siginfo_t의 속성은 다음과 같다.**

si_pid : 자식 프로세스의 PID

si_uid : 자식 프로세스의 RUID

si_signo : 항상 SIGCHLD가 된다.

**si_code**

CLD_EXITED : 정상 종료

CLD_KILLED : 시그널에 의한 종료

CLD_DUMPED : 시그널에 의한 종료로 코어 덤프됨

CLD_TRAPPED : 트랩됨

CLD_STOPPED : 정지됨

CLD_CONTINUED : 재개됨

**si_status : si_code에 대한 부가 정보**

ci_code == CLD_EXITED → 자식 프로세스의 종료 상태

ci_code == CLD_KILLED → 자식 프로세스가 종료한 시그널 번호

**options (OR 연산으로 결합 가능)**

WNOHANG : 준비된 자식 프로세스가 없으면 즉시 반환(논블럭킹)

WNOWAIT : 자식 프로세스의 종료 상태를 읽고 남겨둠(다시 읽기 가능)

WSTOPPED : 정지된 자식 프로세스에 대해 보고

WCONTINUED : 재개된 자식 프로세스에 대해 보고

WEXITED : 정상 종료된 자식 프로세스에 대해 보고

# 10-2

시그널과 세션, 프로세스 그룹

시그널 지연 처리

대체 시그널 스택

### 시그널과 세션, 프로세스 그룹

일반적으로 시그널은 프로세스에 향한다.

터미널에서 CTRL+C를 하면 SIGINT가 발생하고, 프로세스 그룹에 전파된다.

이를 통해 그룹 내 속한 자식프로세스까지 한 번에 종료된다.

**프로세스 그룹**

fork를 통해 생성된다. 프로세스 그룹은 자식 프로세스들을 관리할 수 있도록 만들어진 그룹이다.

쉘의 경우 프로세스는 자신의 PID와 동일한 PGID를 가지는 프로세스 그룹을 생성한다.

PID == PGID인 프로세스는 프로세스 그룹 리더이다. 그룹 리더가 fork하는 경우 자식 프로세스는 같은 프로세스 그룹에 속한다.

자식 프로세스가 새로운 그룹 리더가 되려면 setpgid를 호출해 새로운 프로세스 그룹으로 독립해야 한다.

다른 프로세스 그룹으로 옮기는 경우도 setpgid를 호출해야 하는데, 세션이 같아야만 가능하다.

**세션**

현대 유닉스 시스템에서는 SID로 구분한다.

일반적인 의미로 통신이 시작하는 순간부터 통신이 끄나는 논리적인 구간을 뜻한다.

유닉스에서는 로그인이나 터미널을 통해 통신이 시작된다.

사용자와 연결되는 창구 역할을 하는 프로세스(ex. 쉘) PID와 동일한 SID를 부여 받는다.

PID == SID인 프로세스는 세션 리더이다.

세션 리더가 fork 하면 SID를 상속한다.

키보드 입력 혹은 마우스 입력을 받을 수 있어 사용자와 연결되는 창구 역할을 한다.

SID가 있는 프로세스는 터미널 입력이 가능한 프로세스이다.

터미널 입력 → 프로세스를 제어(제어 터미널) CTRL+C, CTRL+W, CTRL+Z 등

제어 터미널은 같은 세션 내에 있는 모든 프로세스가 돌아가면서 사용이 가능하다.

fg 명령 : 프로세스를 포그라운드로 전환한다. 제어 터미널에 연결된 상태이다.

Bg 명령 : 프로세스를 백그라운드로 전환한다. 제어 터미널에서 잠시 연결해제된 상태이다.

유닉스는 의사터미널(Pseudo Terminal)로 구현되었다. pty로 표현하고, ps 명령어 사용시 TTY 필드에서 확인이 가능하다.

유닉스에서 세션은 사용자와 통신할 수 있는 일련의 프로세스들이 활동하는 논리적으로 구별되는 시스템 내의 가상공간이다.

세션은 유닉스(리눅스)에서 SSH, TELNET과 같은 원격 연결을 하거나, 터미널에 직접 로그인하거나, XTERM과 같은 GUI 프로그램으로 접속할 때 생성된다.

혹은 setsid를 호출해서 새로운 세션을 만들 수 있다. 새로운 세션 리더는 동시에 프로세스 그룹 리더가 된다.

```c
pid_t setsid(void); // 자기가 세션리더가 되기 위해 호출해서 인자가 필요하지 않다
pid_t getsid(pid_t pid);

int setpgid(pid_t pid, pid_t pgid);
pid_t getpgrp(void); // 본인 프로세스 그룹 구하기
pid_t getpgid(pid_t pid); // pid의 프로세스 그룹 구하기
```

**FORK와 프로세스 그룹**

FORK로 맺은 부모/자식 프로세스 관계에서 자식이 만약 프로세스 그룹에서 독립하고 종료하면?

부모 프로세스가 wait를 통해 정리해야만 한다.

**프로세스 그룹과 시그널 전파**

kill 함수에서 PGID에 -1을 곱한 음수형태로 표현하여 전파

killpg 함수를 사용하여 전파

하나의 부모 프로세스가 fork, fork-exec한 다양한 프로세스에 대한 관리

그룹별로 시그널을 통한 제어가 가능하다.

IPC를 이용하여 더 세밀한 작업이 가능하지만, 통신 및 프로그래밍 오버헤드가 발생한다.

간단한 제어는 시그널 전파를 사용하면 간결해질 수 있다.

**부모 프로세스를 제외한 모든 자식 프로세스를 종료하려면?**

부모 프로세스는 SIGTERM 신호를 블록 혹은 무시하고

자신이 속한 그룹에 SIGTERM을 보낸다.

부모는 SIGTERM이 블록되고 자식들은 SIGTERM으로 인해 종료될 수 있다.

**데몬 프로세스와 세션**

웹 서버와 같은 프로세스는 데몬 프로세스이다. 로그인 쉘을 종료(세션 종료)해도 계속 실행된다.

데몬 프로세스는 독립 세션을 갖고, 제어 터미널을 갖지 않는다.

표준 입출력을 사용하지 않는다(stdin, stdout, stderr를 모두 닫는다)

### 시그널 지연 처리

**시그널 블록**

특정 코드 구간에서 도착한 시그널을 지연시킨다. 프로그램 실행이 방해받으면 안되는 구간(Critical Section)을 보호한다.

예를 들어 느린 I/O 중에 EINTR이 발생하는 것을 막는다. 타임 아웃을 사용한 작업 중 시그널 개입을 막는다.

sigprocmask는 스레드에 안전하지 않은 함수이므로, 스레드 환경에서는 pthread_sigmask를 사용해야 한다.

sigpending

sigsuspend

```c
//블록된 시그널 읽기, set : 읽어온 블록된 시그널을 저장한다.
int sigpending(sigset_t *set);
```

시그널도 원자적 실행에 영향을 받는다. 시그널 핸들러에서 전역 변수를 다룰 때 주의해야한다.

```c
//volatile : 최적화하지 말 것, sig_atomic_t : 시그널 안전 타입 (정수형) -> 읽거나 쓰는 단일 행동에 원자적 실행을 보장한다, 동기화용 변수로 많이 사용.
// 다른 전역 변수를 읽어도 안전한지 확인하는 용도(스핀락 형태 구현)
volatile sig_atomic_t variable;
```

pthread와 같은 멀티 스레드를 사용할 경우 → pthread에서 제공하는 시그널 처리 함수를 사용해야 한다.

### 대체 시그널 스택

sigaction 함수의 sa_flags 에 SA_ONSTACK을 설정해서 사용한다.

개발된 의도는 디버깅에 유용하다.

```c
//대체 시그널 스택 지정 ss: 새로 할당한 시그널 스택, oss: 이전 스택 설정 백업
int sigalstack(const stack_t *ss, stack_t *oss);

typedef struct sigalstack{
	void *ss_sp; // 대체 스택으로 지정할 메모리 공간
	size_t ss_size; // 스택의 공간 크기 지정
	int ss_flags; // SS_ONSTACK -> 백업된 oss 에만 지정됨, 이전 시그널 스택의 사용 중임을 의미, 사용자가 설정할 수 없음 / SS_DISABLE : 대체 시그널 스택 비활성화(프로세스 스택사용)
} stack_t;

```

# 10-3

MQ 동작 방식

**MOM(Message Oriented Middleware)**

비동기 메시지를 사용하는 다른 응용 프로그램 사이에서 데이터를 송수신할 수 있는 계층 제공

**MiddleWare**

middleware → 이기종 구성 요소를 통합하는 계층, 동종 구성 요소 간 통신과 이종 구성 요소 간 통신의 차이를 극복하는 것

미들웨어 범주

RPC(Remote Procedure Call) 기반 → 함수의 호출을 원격에 있는 시스템에서 찾아 수행

ORB(Object Request Broker) 기반 → 객체를 이기종 환경에서 공유

MOM(Message Oriented Middleware) 기반 → 분산 환경에서 메시지를 교환

RPC & ORB → 밀접한 연결 관계, 동기식 방식(요청 → 처리 → 응답)

MOM → 느슨한 연결, 비동기식 방식(요청, 처리, 응답이 분리되어 각각 개별적 처리 방법에 따라 진행)

**메시지큐의 장점**

비동기 → 큐에 데이터를 넣고, 필요할 때 사용

비동조 → 응용(로직)에서 분리

탄력성 → 일부의 실패 , 전체에 영향을 주지 않음

과잉 → 실패 → 재실행 가능

보증 → 작업의 처리 확인 가능

확장성 → 다수의 프로세스 연결 가능

**대용량 데이터 처리, 비동기 데이터 처리, 채팅**

# 11-1

I/O 멀티플렉싱(다중 입출력)

**다중 입출력  통신**

기본적인 프로그래밍 설계

순차적으로 작업을 처리하도록 설계 → 논리적으로 데이터가 어떤 작업을 거쳐서 가공되는지 정의, 한 번에 한개의 데이터 흐름만 있다고 가정하여 단순화

실제 구현

한 번에 한개의 데이터 흐름만 있지 않다.

중간에 병목현상으로 지연이 발생하면?

다음의 모든 작업이 지체되고 높은 지연(Latency)가 발생된다.

지연을 해결하기 위한 구조로 설계가 필요하다.

프로그램에서 여러 개의 프로세스와 ㅌ오신하기 위해서 FIFO, 소켓 등과 읽고 쓰기 중

각 연결에 통신 순서를 결정한다.

만약 10번 연결에서 수신된 버퍼가 없을 때 read 혹은 recv를 하고 있다면 블록킹 모드로 I/O를 하면 무한 대기하고,

넌블록킹 모드로 I/O를 하면 EAGAIN 에러가 발생한다.

넌블록킹을 하면 다음 순서의 연결을 차례대로 읽을 수 있다. 

모든 연결을 넌블록킹으로 하는 것이 가능하지만 프로그램에서 에러 처리를 해야하는 문제가 있다.

에러를 피하면서 연결들(다중 입출력)에서 데이터가 발생한 순서대로 처리하고 싶다 → **I/O 멀티플렉싱 기법 사용**

**넌블럭킹과 비동기적 I/O**

저수준 입출력 사용시 얻는 이점

넌블럭킹 I/O → 블럭킹 되지 않는 것, 설계 시 데이터 처리 순서를 고려한 것(에러 처리 후 재시도)

비동기적 I/O == AIO (Asynchrounse I/O) → 리얼타임 확장 표준에 포함되었다. 고성능 시스템에 적용하기 위한 기능이다.

작업의 요청과 작업의 결과 처리 흐름을 분리한다. 병렬적 실행이 가능하도록 디자인되었다.

작업 실행의 완료가 달라도 되고, 작업의 결과 처리를 위해서는 별도의 알림 및 실행 처리 기능이 필요하다.

**용어의 차이**

동기적(synchronuse) 

시간 혹은 공간 등을 일치하는 것, 실행 단위 마다 적용,

프로그램 코드에서는

동기적 호출 : 함수의 호출 부분에서 응답(결과)를 받음

비동기적 호출 : 함수의 호출 부분에서 응답이 오지 않고, 다른 함수 혹은 위치에서 응답을 받아 처리함

동기화(synchronization) : 실행 작업 자체를 일치시키는 것

**I/O 멀티플렉싱**

핵심 : I/O 이벤트를 감지 → select, poll 같은 함수를 사용한다.

소켓, 입출력 장치들과 연결한다. 입력 및 출력 이벤트 감지 기능이 있다. 이 이벤트가 발생할 때만 처리하고 없으면 다른 작업을 한다.

특정 이벤트를 감지하려면?

폴링하면서 대기한다 → I/O poller 혹은 I/O 멀티플렉서

개선된 폴링 방법 → 리눅스 epoll, BSD kqueue, 솔라리스 /dev/poll, 윈도우 계열 IOCP

I/O 멀티플렉서 매커니즘

select : 범위를 가장 큰 파일기술자를 지정하고 이 값이 크면 오버헤드 발생한다.

poll : 정교한 핸들링이 간으하다 중규모(500~1000개 ㅇ녀결)에서 주로 사용한다.

epoll, /dev/poll, kqueue : 더 높은 성능, 단 비표준(이식성 문제가 있다)

동작은 레벨 트리거(select, poll)와 에지 트리거(epoll 등) 방식

**select**

```c
//nfds : 감시할 가장 큰 파일 기술자 +1로 지정, readfds, writefds, exceptfds :파일 기술자 집합, 읽기 가능 이벤트, 쓰기 가능 이벤트, 예외 이벤트 감시
//fd_set에 비트 1 설정 -> 감시를 지정한다.
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec *timeout, const sigset_t *sigmask);

void FD_CLR(int fd, fd_set *fdset); //집합을 초기화 
void FD_SET(int fd, fd_set *fdset); //파일기술자를 집합에 등록
void FD_ZERO(fd_set *fdset);        //파일기술자를 집합에서 제외
void FD_ISSET(int fd, fd_set *fdset); //파일기술자가 집합에 포함되어있는지 확인
```

버퍼의 수준을 감지한다. 버퍼의 수준은 바이트 규모 ≥ 1 여부

레벨 트리거 : 이벤트 발생 여부가 버퍼의 상태로 결정된다.

select는 소규모 연결에 적합하다. 만약 감시하는 파일 기술자의 값이 크면, poll을 사용하자, 그리고 매크로를 사용하여 판별하여 직관적이지 않다.

select vs pselect

타임아웃 설정방법

1. select : struct timeval → 마이크로초
2. pselect : struct timespec → 나노초

함수 호출에 의한 블럭킹 중 시그널 처리

1. select : 시그널 발생하면 EINTR 에러가 발생한다. 전역적인 시그널 블록 마스크가 필요하다.
2. pselect : 함수의 인자로 시그널 블록 마스크를 사용한다.

이벤트의 발생 : 블럭킹/ 논블럭킹 모드에 관계 없이 감지한다.

실무 I/O (read/write)에서는 블럭킹보다는 넌블럭킹을 선호한다.

readfds: 수신 버퍼에 데이터가 도착한 경우, 수신 버퍼에 종료(해제)가 발생한 경우, 소켓인 경우 SYN 패킷이 있는 경우

writefds: 송신 버퍼에 빈공간이 생긴 경우(flush에 해당하는 이벤트), 반대편에서 종료(연결 해제)한 경우, 스트림 연결 시 데이터 전송이 가능한 경우

주의 readfds, writefds, exceptfds는 입력이면서 출력이다. 

select 수행 후 변경된다. readfds는 읽을 수 있는 파일기술자만 set, writefds는 쓰기 가능한 파일기술자만 set, exceptfds는 예외가 발생한 파일기술자만 set한다.

timeout

이벤트가 감지를 기다리는 타임아웃(절대시간이 아님)

NULL로 지정하면 무한정 기다리기,

출력시 변경 : 5초의 타임아웃을 설정하고, 2초 후에 이벤트가 발생하면 남은 대기시간이 변경되어 기록된다.

반환값:

1.  타임아웃이 없을때
    1. 성공 → 양수
    2. 실패 → 음수
2. 타임아웃이 있을 때
    1. 성공 → 양수
    2. 타임아웃 되면 0

**POLL의 사용**

```c
//반환값 : 이벤트가 발생한 파일기술자의 개수
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
// GNU LINUX Only
int ppoll(struct pollfd *fds, nfds_t nfds,
					const struct timespec *timeout_ts,
					const sigset_t sigmask);

struct pollfd{
	int fd; //감시할 파일 기술자
	short events; //requested : 감시할 이벤트 지정, OR 지정 가능
	short revents; //returned : eventes 에서 지정한 감시 이벤트가 발생하면 포함된다, 혹은 POLLERR, POLLHUP, POLLINVAL 에러 발생시 포함된다.
};

//감시할 이벤트의 종류
POLLIN //읽기 버퍼의 데이터 수준 감시
POLLPRI //우선 순위 높은 입력 이벤트 (예: 예외상황 등)
POLLOUT //쓰기 버퍼 사용 가능 감시
POLLERR //에러 감지(revents 전용)
POLLHUP //닫힌 연결에 쓰기 시도 감지(revents 전용)
POLLINVAL //유효하지 않은 파일 기술자 지정 에러(revents 전용)
```

nfds : fds 배열의 크기

timeout : 밀리초 단위 → 실수로 NULL을 넣지 않도록 한다. NULL을 넣으면 0으로 캐스팅 되어 즉시 타임아웃 될 수 있다. 타임 아웃을 사용하지 않으려면 -1을 사용한다.

**SELECT VS POLL**

이벤트 마스크 집합 설정 방법에서 차이가 많이난다. → poll 방식이 훨씬 더 직관적이다.

감시할 파일 기술자의 지정 → select가 검사하는 파일 기술자 지정이 쉽다. 

select는 내부적으로 0~ 최대 파일 기술자를 반복하며, 마스크를 모두 확인한다.

poll은 지정된 배열의 수 만큼만 확인한다.

**코드의 변화 말고 성능의 차이는 있을까?**

앞에서 말한 select에서 파일기술자가 너무 큰 경우를 제외하면, 거의 없다.

감시해야 할 파일 기술자가 많은 경우에도 차이가 없다.

**SELECT/POLL과 넌블럭킹 I/O**

SELECT/POLL은 비싼 비용의 함수이다.

동기적 실행 함수 → 블록킹이 발생한다.

모든 읽기 가능 여부를 확인하는 것이 병목 현상을 일으킨다.

SELECT/POLL과 넌블럭킹 모드 I/O 병행 사용

긴 대기가 필요한 경우 SELECT/POLL로 진행한다.

연속된 데이터 스트림을 처리하는 경우

select/poll 개입 없이 연속적으로 read 하거나 write를 수행한다.

이 때, 블럭킹 모드를 사용하게 되면, 무기한 대기할 수 있다.

대기가 발생하지 않도록 넌블럭킹 모드를 사용한다. 넌블럭킹에 의한 에러(EAGAIN) 발생 시, 긴 대기 상태로 전환된다.

select/poll로 여러 I/O를 동시처리하여 대기 시간을 줄이고, 효율을 높이려면 넌블럭킹 모드를 사용한다.

**SELECT/POLL의 문제점**

고성능 I/O 멀티플렉서

SELECT / POLL 의 문제점 → 시스템 호출에서 함수 인자를 처리한다.

시스템 호출에서 사용하는 함수 인자 값이 커지면 콜 호출 비용이 늘어난다.

I/O 의 병목을 줄이려는 목적,

SELECT / POLL의 추가 오버헤드가 발생한다 → 커널에 재진입하는 비용과 커널에 메모리를 복사하는 비용이 추가된다.

메모리 복사는 CPU와 메모리 대역폭에 따른 버스 지연이 발생한다.

epoll은 poll에서 메모리 복사 비용이 큰 것을 해결한다.

# 12-1

멀티플렉싱 epoll

**넌블럭킹 I/O**

대부분의 I/O 처리 → 기본 : 블록킹 처리

예제나 동작 확인 테스트 혹은 소규모 처리에서 블록킹 처리를 한다.

대규모 처리의 실무에서는 넌블록킹 모드를 사용한다.

주의점

넌블록킹 모드를 사용하면 논리적 버그인 잠정적 무한루프 문제를 일으킬 수 있다. 예상보다 훨씬 느리게 동작할 가능성이 높아진다.

넌블럭킹과 동기/비동기를 혼동하지 말자.

**넌블록킹과 블록킹**

분류기준 대기 

블록킹 → 대기 , read하거나 write 할 때, 완료될 때까지 대기, 함수가 반환할 때, 성광과 실패를 알 수 있다.

넌블럭킹 → 대기 안함, 어떤 요청을 하면, 성공이든 실패든 일단 즉시 처리 후 반환한다. 함수가 반환되면 성공, 실패, 부분성공의 상태가 존재 가능

**동기와 비동기**

분류 기준은 순서

동기 → 순서대로 처리 보장

비동기 → 순서대로 처리를 보장하지 않음(out-of-order)

일반적인 시스템 프로그래밍의 입출력 함수는 모두 ‘동기’ 모델이다.

비동기식은 시그널, AIO, 스레드(Task 단위로는 비동기, Task 내의 기능들은 동기 → 스레드의 동기/비동기 여부는 기준 관점에 따라 다를 수 있다.)

시점에 따라 달라지는 예시:

시스템 프로그래밍 함수들 : 사용자 공간(User Space)에서는 동기적으로 동작하고, 커널 공간(Kernel Space)에서는 비동기로 동작한다.

동기/비동기 구분을 엄격하게 하려면?

어느 수준 혹은 어떤 시점인지를 명확하게 해야 한다. → 생략하면 보편적으로 시스템 프로그래밍 수준(User Space)으로 간주하여 이해한다.

사용한 라이브러리는 비동기식이지만 라이브러리가 사용하는 함수 단위에서는 동기일 수도 있다.

**블록킹 모드의 약점**

일반적으로 파일, 소켓, IPC 등 모든 I/O 함수는 블록킹 모드가 기본이다.

에러가 발생하지 않는다면, 원하는 동작을 할 때까지 대기한다.

read는 읽기가 가능해질 때 까지 대기하고, write는 쓰기가 가능해질 때 까지 대기한다.

대기시간이 기약 없이 길어질 수 있다.

read하려고 하는 데, 상대방이 데이터를 보내지 않게 되면 언제 read가 끝날지 모른다.

결국 프로그램은 다른 일을 할 수 없고, 대기만 해야된다.

(1:1 통신이나, 외부의 개입이 없는 경우는 문제가 없음 → 실무에는 잘 없음)

```c
//넌블럭킹으로 전환하기
int fcntl(int fd, int cmd, .../* arg */);

//열고 있는 파일기술자를 넌블록킹으로 변경할 수 있다.
flag_old = fcntl(fd, F_GETFL); // 기존 설정, 모드 읽어오기
if(fcntl(fd, F_SETFL, flag_old | O_NONBLOCK) == -1){
	//error
}

//리눅스에서는 F_SETFL 명령이 다음 플래그에만 영향 받도록 설계된다. O_ASYNC, O_APPEND, O_NONBLOCK
if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1){
	//error
}

// 반대로 블록킹으로 전환하려면
flag_old = fcntl(fd, F_GETFL); //기존 설정, 모드 읽어오기
if(fcntl(fd, F_SETFL, flag_old & (~O_NONBLOCK)) == -1{
	//error
}

// 일반 파일은 열 때, O_NONBLOCK을 지정한다. open혹은 개시할 때 지원하는 경우 사용한다. 
// open 혹은 개시 결과 파일기술자를 사용하지만, O_NONBLOCK으로 시작할 수 없다면 fcntl을 이용해 전환할 수 밖에 없다.
if(open(path_loc, O_APPEND | O_NONBLOCK) == -1){
	//error
}
```

**넌블록킹으로 하면 빨라지나요?**

입출력은 넌블럭킹 모드와 성능과 관계가 없다.

DISK vs 네트워크

일반적으로 디스크가 네트워크 보다 반응이 빠르다.

**DISK I/O**

또한 mmap으로 처리하여 대기시간을 없앨 수 있으므로 넌블럭킹을 사용할 필요성이 크게 높지 않다.

블록킹 모드에서 시그널로 인한 인터럽트

I/O 요청 작업 취소, 에러 발생처리, errno에 EINTR 설정 → 프로그래머가 다시 요청하도록 함

**I/O 요청 작업 취소가 할 수 없는 상황이 발생할 수 있다면,**

요청 → 네트워크 SYN 패킷 발생 → 인터럽트, 함수는 취소된다

이미 보낸 SYN 패킷을 취소할 수는 없다.

**취소가 불가능한 상황이 존재한다면?**

재시작을 하지 말아야한다. 시그널에 의한 자동 재시작 뿐만 아니라 프로그래머도 하면 안된다.

앞의 예에서 반복적으로 재시도하면 SYN 패킷을 반복적으로 보내는 플러딩 공격이 된다.

**시그널로 인한 인터럽트가 발생하지 않도록 한다.**

**넌블럭킹으로 전환한다면**

사용하는 함수의 에러의 의미를 파악해야 한다.

성공/실패 외에 성공과 실패의 사이에 있는 상태도 에러(EAGIN)로 표현될 수 있다.

**블럭킹 모드와 넌블럭킹 모드에 따라 서로 다른 문제점이 존재한다.**

**블럭킹 모드의 문제**

원하는 크기의 데이터를 읽지 못하면 무한 대기 상태로 빠진다.

read_nbyte(fd, buf, 5000) → 실제로 읽을 수 있는 데이터가 4999 바이트 뿐이라면?, 10초 뒤 1바이트가 더발생하면? → 10초 대기시간 발생

**넌블럭킹 모드의 문제**

read_nbyte(fd, buf, 5000) → 4999바이트만 읽을 수 있는 상황? EAGIN에러가 발생할 것이며, 에러 루프를 계속해서 반복하게 된다.

대기하는 것은 같지만 무한 루프가 발생해 CPU를 의미 없이 낭비하고 10초 정도면 수십만번 에러 처리를 하게된다.

타임아웃을 주고 해결하면? → 타임아웃으로 종료하지만, 그 시간은 CPU를 무의미하게 낭비하는 게 동일

read를 블록킹모드로 하고 타임아웃을 짧게 반복하면? → 세밀한 타임아웃을 반복적으로 사용하는 건 오버헤드가 커짐(프로그래밍 복잡하고, 커널 사용 횟수 증가)

**구조적으로 잘못 된 함수의 설계가 문제**

**예정한 크기를 읽기 위한 루프가 문제다**

1. 수신 데이터를 보관하는 버퍼(큐)를 만들고
2. 데이터가 수신되면 버퍼에 넣는다.
3. 버퍼의 데이터가 예정한 크기가 되면 그 만큼을 처리한다. → 처리 방법은 이를 하는 다른 함수를 호출하는 방법을 생각해볼 수 있다.
4. 이렇게 처리하는 것이 비동기 방식

**EPOLL**

SELECT vs POLL

넌블럭킹 모드를 사용하더라도, 매번 에러를 처리하는 것은 오버헤드이다.

read 혹은 write 할 수 있을 때만 처리하면 에러 처리 오버헤드를 줄일 수 있다.

poll은 500개 ~ 1000개 규모의 적은 파일 기술자 대상에 적합하다.

파일 기술자가 많아질수록 느려진다. 대규모 프로세스(로컬, 원격 등)간 통신에서는 문제가 된다.

이를 해결하기 위한 방법으로 epoll이 등장한다. 다만 아직 비표준 방법이므로 poll 사용법도 알아야 한다.

**EPOLL의 특징**

**stateful함수**

poll과 달리 호출할 때마다, 파일 기술자 정보를 전달하지 않는다.

select/poll은 커널에게 파일 기술자 어떤 것을 사용할지 계속 알려주어야 하므로 stateless 함수라고 할 수 있다.

epoll은 파일 기술자를 커널에 등록, 해제, 변경하는 함수가 있다.

epoll은 이벤트를 감시하는 함수가 별도로 존재한다.

edge 트리거 지원

select / poll은 레벨트리고, epoll도 기본은 레벨트리거로 동작한다.

트리거 : 어떤 조건을 만족하는 지 감지하는 것이다.

레벨 트리거 : 어떤 값, 상태가 **특정 수준**에 이르는 지 감지하는 것이다. 배터리 잔량 15% 미만

엣지 트리거 : 이전 상태에서 **변화**가 생겼는지 감지하는 것 : 전원 상태가 충전 → 배터리 모드로 변경

**EPOLL / POLL 의 레벨 트리거**

poller (epoll, poll)가 감지하는 대상이 수신버퍼일 때,

1바이트라도 있으면 트리거 → 레벨 트리거

poller를 호출, FD 1번에 이벤트 발생 확인 → 이번에 처리하지 않음

다음, poller 호출 → FD 1번에 **변화가 없음**, 그러나 1바이트가 처리되지 않고 남아 있어, 다시 이벤트가 발생한다.

**EPOLL의 엣지 트리거**

poller(epoll, poll)가 감지하는 대상이 수신버퍼일 때,

이전 보다 1바이트라도 증가하면 트리거 → 엣지 트리거

Poller를 호출

FD 1번에 이벤트 발생 확인 → 이번에 처리하지 않음

다음, poller 호출

FD 1번에 변화가 없음

**상태 변화가 없음 (추가 데이터 0), 이벤트 발생 안함**

**EPOLL의 생성**

```c
// epoll 생성하기
int epoll_create(int size);
int epoll_create1(int flags);

// epoll 제어하기, 감시할 파일 기술자와 감시 이벤트를 등록/해제 한다.  
// epfd: epoll 파일 기술자, fd : 감시할 파일 기술자
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

//op : 
EPOLL_CTL_ADD (fd를 epoll 파일 기술자에 등록), 
EPOLL_CTL_DEL (fd를 epoll 파일 기술자에서 제거, 감시 목록에서만 제외된다, 파일 기술자가 닫히는건 아님)
EPOLL_CTL_MOD (epoll 파일 기술자에서 fd에 대한 감시 이벤트를 변경한다)

//event -> 등록(EPOLL_CTL_ADD)하거나 교체(EPOLL_CTL_MOD)인 경우 사용한다, 감시할 이벤트는 32bit 정수, poll의 이벤트와 유사, poll 이벤트의 정의 앞에 E만 붙이면 된다.
//data는 공용체 -> 감시 대상 파일 기술자를 가리킨다.
struct epoll_event{
	uint32_t events; //Epoll events
	epoll_data_t data; // User data
}

typedef union epoll_data_t {
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u54;
} epoll_data_t;

//EPOLL의 이벤트 수신
// events : 감지된 이벤트 배열이 저장됨, 저장할 수 있는 배열의 크기를 maxevents로 지정, timeout : 음수 : 무한대기, 0: 넌블럭킹 동작, 양수: 주어진 시간만큼만 대기
// 반환값 -> 이벤트가 발생한 파일 기술자의 개수 반환
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
//epoll_pwait는 epoll_wait 시 블록할 시그널을 지정한 마스크가 추가된다.
int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask);

```

커널에 객체 공간을 확보, 반환 값은 epoll 파일 기술자, 사용하지 않으면 close 함수로 닫아야한다. (POSIX방식)

epoll 파일 기술자는 다른 파일 기술자와는 다르다. read / write로 사용할 수 없다.

size : epoll에 등록할 수 있는 파일 기술자의 개수, 커널 2.6.8 부터는 동적으로 결정한다, 커널에서 직접 관리한다(통상 1을 줌)

flags : EPOLL_CLOEXEC, CLOSE-ON-EXEC 만 사용 가능하다, exec 사용 시 epoll 종료하기 (통상 자식 프로세스 생성 후 exec 하는 경우, 안전장치)

epoll의 제한

커널 설정 fs.epoll.max_user_watches에 의해 제한을 받는다.

사용자(계정)별 감시 가능한 파일 기술자의 초대 개수

기본 값은 Low memory의 4%

X86(32비트)는 free low memory * 0.04

X86_64는 low memory 구분 안함 → total free memory * 0.04

파일 기술자 한 개 감시하는데 필요한 메모리는 90~160 bytes

poll과 epoll의 차이

poll은 어떤 파일기술자에서 이벤트가 발생했는 지 알 수 없다. 감시하는 모든 파일 기술자를 확인해야 한다.

epoll은 events 구조체에 발생한 파일 기술자가 명시되어 있다. events의 크기가 반환 값이라 생각하면 된다.

**리얼타임 확장**

리얼타임 시그널

리얼타임 이벤트

비동기적 I/O (AIO, Asynchronouse I/O)

기존의 작업들을 비동기적 실행으로 변경하거나 돕는 기능들 추가

동기적, 비동기적 시점에 따라 다름

비동기적 I/O를 사용하는 이득

I/O 대역폭이 분리되어 있거나, 여유가 있다면, 지연시간을 줄일 수 있다.

시스템의 효율성(Utilization)을 높일 수 있다. Latency Hidinig 효과

비동기적 시그널 이벤트 통지 → 어떤 함수의 시작이나 완료 혹은 어떤 이벤트가 발생했을 때, 추가적으로 콜백을 발생시키는 메커니즘

디자인 패턴 : 콜백, 옵저버

POSIX 메시지 큐의 mq_notify가 대표적인 예시이다.

리얼타임 시그널 → 전통적인 UNIX 시그널 메커니즘을 확장했다. RTS, RT 시그널이라 부른다.

**달라진 점 : 새로운 리얼타임 확장 I/O 관련 함수 사용시 RT 시그널로 보고 받을 수 있다 → 비동기적 시그널 이벤트 통지**

**시그널 큐를 가진다. → 전통적인 시그널은 중복 시그널을 구본하지 못한다. 대개의 경우 32개 이상 큐에 쌓을 수 있다.**

이벤트 통지 기능을 이용해서 I/O 멀티플렉싱을 대체할 수 있다.

select/poll/epoll과 같은 poller를 사용하지 않을 수 있다. poller는 이벤트가 있는지 검사하는 함수이다.

**이벤트를 따로 검사하지 않고 이벤트 발생 즉시 처리가 가능하다.**

단점 :

시그널 처리 방식의 프로그래밍 → 기존의 동기식 프로그래밍과 구조적으로 다르다, 기존에 잘 구성된 라이브러리와 호환되지 않을 수 있다.

시그널 큐의 길이 제한이 있다 → 무작정 큰 스케일을 가지는 프로그램에 적용할 수는 없다. 기존의 문제가 자동으로 해결되지는 않는다.

# 13-1

리얼타임 시그널

**SIGEVENT : 비동기적 시그널 이벤트 통지의 핵심 → 이벤트를 처리하는 방법을 정의**

```c
typedef struct sigevent{
	int sigev_notify; //SIGEV_SIGNAL(이벤트 통지로 시그널 발생), SIGEV_NONE(이벤트 통지 사용 안함), SIGEV_THREAD(이벤트 통지로 스레드를 생성하여 처리)
	int sigev_signo;  //sginal number
	union sigval sigev_value; //signal value
	void (*sigev_notify_function)(union sigval); //통지 스레드 함수
	pthread_attr_t *sigev_notify_attributes; // 통지 스레드 속성
};

typedef union sigval{
	int sigval_int; //int 값을 전달하는 경우
	void *sigval_ptr; //포인터를 전달하는 경우
} sigval_t;

//리얼타임 시그널 수신하기, 시그널이 올 떄 까지 프로그램을 블록한다.
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *restrict timeout);
```

sigwait vs sigwaitinfo / sigtimedwait

sigwait은 실패시 에러 코드(양수)를 반환

sigwaitinfo와 sigtimed wait은 실패시 -1 반환 → errno에 에러코드 설정

SIGQUEUE 함수

시그널을 큐를 통해 전송한다 

메시지 큐와 달리 시그널을 차례대로 전송하는 것이 목적이다.

중복된 시그널을 전송 가능하다. 시스템의 큐의 크기 설정 제한을 받는다. (리눅스 구현 별로 다르지만 대체로 32개 이상이다)

시그널 + 추가데이터 전달

추가 데이터는 siginfo의 sigval 공용체러 존달(간단한 정수나 포인터 값 전달이 가능하다)

주소 값인 포인터는 다른 프로세스에서는 의미 없음(멀티 스레드 환경에서 사용)

**시그널 큐로 시그널 보내기**

```c
int sigqueue(pid_t pid, int sig, const union sigval value);

//sigval 수신자 측에서 siginfo_t의 si_value로 전달된다.
typedef union sigval{
	int sival_int; // int 값을 전달하는 경우
	void *sival_ptr; // 포인터를 전달하는 경우
} sigval_t;
```

**리얼타임 시그널 I/O 멀티플렉싱**

리얼타임 시그널을 사용하여 I/O 멀티플렉싱을 구현할 수 있다.

select, poll, epoll 대신 사용 가능하지만, RTS는 리눅스의 확장이다. 유닉스에서는 아직 사용할 수 없다.

RTS I/O 멀티플렉싱

지연된 시그널 처리 방법과 유사하다.

특정 팡리 기술자에 입출력이 생기면 시그널을 발생시키기 → fcntl을 통해 설정이 필요하다.

```c
// 1. 파일 기술자를 넌블록킹 모드와 비동기 입출력 가능 상태로 변경하기
if(flags = fcntl(fd, F_GET_FL)) == -1){
	//에러 발생
}

if(fcntl(fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC) == -1) {
	//에러 발생
}

// 2. 파일기술자에 어떤 이벤트가 발생했을 때 통지할 시그널 번호 지정하기
// 통지받을 RTS 시그널을 i_sig라고 할 때
if(fcntl(fd, F_SETSIG, i_sig) == -1){
//에러 발생
}

// 3. 파일기술자에 입출력이 발생했을 때, 시그널을 받을 프로세스의 PID를 지정하기
// 자기 자신에게 통지하려는 경우
if(fcntl(fd, F_SETOWN, getpid()) == -1){
	//에러 발생
}
```

주의

sigprocmask 등으로 시그널을 블록하여 지연하지 않으면 기본 시그널 핸들러가 동작한다.

멀티스레딩 환경(pthread)에서는 pthread_sigmask를 사용한다.

sigwaitinfo / sigtimedwait를 사용하여 이벤트를 얻는다.

**디렉터리 이벤트 감시**

리얼타임 시그널과 GNU 리눅스 확장의 조합

디렉터리의 변화를 실시간으로 감시할 수 있다.

fcntl에 F_NOTIFY 옵션을 이용한다(옛날 방법)

→ 파일의 읽기, 쓰기, 생성 등의 변화를 감시할 수 있다.

→ 최근에는 inotify로 리눅스 커널에서 제공한다.

```c
//디렉터리 변경 이벤트 설정
if(fcntl(fd, F_NOTIFY, DN_ACCESS|DN_MODIFY|DN_MULTISHOT) == -1){
	//에러 발생
}

//감시하는 디렉터리에 존재하는
DN_ACCESS (파일이 읽혀짐)
DN_MODIFY (파일이 변경됨)
DN_CREATE (파일이 생성됨)
DN_DELETE (파일이 삭제됨)
DN_RENAME (파일 이름이 변경됨)
DN_ATTRIB (파일 속성이 변경됨)
DN_MULTISHOT (통지 기능을 1회 사용하고 제거안함)
```

MULTISHOT을 지정하지 않으면, 최초 한번만 통지한다.

이벤트는 si_band로 전달되며, POLLIN, POLLRDNORM, POLLMSG가 발생한다.

**리얼타임 시계**

POSIX 리얼타임 확장

기존의 시계보다 더 많은 기능을 가지는 시계 정의

재진입성 함수로 만들어, 멀티 스레드에 안전하다.

전통적인 유닉스 시계 그리고 타이머

BSD와 초기 System V에서 유래했다. 알람 시그널 + 전역 변수로 구현되었다.

시그널 개입이 빈번하거나 멀티 스레드 환경에서 타이머 변수를 덮어쓰는 등의 문제가 발생한다(전역변수 사용의 문제)

SUS 표준안에서는 스레드 안전을 명시한다 → 락 메커니즘으로 보호(동시접근만 막는다), 요구사항일 뿐

전통적인 유닉스 시계

sleep: 초 단위 프로세스 재우기, usleep: 마이크로초 단위 프로세스 재우기

alram : 알람 시그널, getitimer, setitimer: 구간 타이머 (제거 예정)

```c
// 리얼타임 시계 관련 함수 usleep -> nanosleep
int nanosleep(const struct timespec *req, struct timespec *rem);
int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain);

//시계와 정밀도
int clock_getres(clockid_t clockid, struct timespec *res); //리얼타임 시계 구현의 정밀도 얻기, 나노 초까지 표현 가능하지만, 시계는 큰 단위만 지원할 수 있다.
int clock_gettime(clockid_t clockid, struct timespec *tp); //지정한 시계의 시간 구하기
int clock_settime(clockid_t clockid, const struct timespec *tp); //지정한 시계의 시간 변경하기

clockid_t
CLOCK_REALTIME (시스템 전역의 실제 시계 사용 Unix Epoch 시간)
CLOCK_MONOTONIC (단조 시계, 어떤 시각을 기준으로 흐른 시간 측정)
CLOCK_MONOTONIC_RAW (하드웨어 기반의 단조 시계)
CLOCK_PROCESS_CPUTIME_ID (프로세스 단위 CPU 사용 시간 측정 시계)
CLOCK_THREAD_CPUTIME_ID (스레드 단위 CPU 사용 시간 측정 시계)

//타이머 생성 clockid : 시계 유형, sevp: 리얼타임 시그널 이벤트, Timerid : 시계 식별자
int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
//타이머 제거
int timer_delete(timer_t timerid);

//타이머 설정 flags: TIMER_ABSTIME: 절대시간 사용, 혹은 0,  value : 타이머 지정, ovalue: 이전 타이머 설정 백업
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
int timer_gettime(timer_t timerid, struct itimerspec *curr_value);

struct itimerspec{
	struct timespec it_interval; //간격  : 0보다 큰 경우 해당 시간마다 타이머 만료된 경우 호출한다. 0인 경우 타이머를 무효화하고 타이머는 일회성으로 작동한다.
	struct timespec it_value; //초기 만료 : 0보다 큰경우 초기 타이머 만료 시간을 설정한다, 0인 경우 이미 설정된 타이머가 있는 경우를 해제한다.
}
struct timespec{
	time_t tv_sec; //초
	long tv_nsec; //나노 초
}

//CPU 시계
if(clock_getcpuclockid(0, &clock_cpu) == -1){
	//에러 발생
}
clock_gettime(clock_cpu, &ts); // 현재 CPU 시간을 ts에 저장
clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); //현재 CPU 시간을 ts에 저장
```

CPU 시계

IEEE 표준 1003.1d-1999 Advanced Realtime extensions에서 추가

프로세스나 스레드가 사용한 CPU 시간을 알 수 있게 함

각 프로세스나 스레드가 소모한 실제 CPU 시간을 측정함 → 로드 밸런싱, 프로파일링 시 유용하다

CPU 시계에 대한 해석 주의 ⇒ CPU 시간이 높다 ≠ 효율적이다.

너무 잦은 측정은 왜곡될 가능성이 있다(시스템 호출이기 때문)

# 13-2

비동기적 I/O (AIO)

기존의 I/O 처리보다 향상된 기능 + 리얼타임 시그널 이벤트 기능

read, write → 소켓, 파이프 모두 사용이 가능

AIO → 디스크, 블록 장치로의 입출력에만 사용이 가능하다.

비동기적 실행 : 어떤 일을 시작을 지시하는 함수, 결과를 확인하는 함수

작업지시 함수의 완료 → 작업을 시작하도록 하는 것이 완료됐다는 의미, 작업의 성공/실패를 의미하지 않는다.

**장점**

I/O 작업을 명령하는 시점과 완료하는 시점 사이에 다른 작업을 할 수 있다.

논블럭킹과 비교 → **논블럭킹은 작업 시작 가능과 불가능을 구분한다.**

**작업이 시작되면 I/O의 완료를 기다린다** → CPU는 유휴 상태

**AIO는 작업 시작 요청 후 즉시 시작함수를 종료한다**

I/O는 백그라운드(커널에서 수행)로 진행한다. **즉 I/O 동작을 기다리지 않는다.**

만약 AIO 요청후 다른 일을 전혀 하지 않는다면?

CPU는 유휴 상태 → 사용하지 않는 것과 같다, I/O 성능을 향상 시키는 것이 아니다.

또 I/O 요청을 하면 ?

I/O는 대개의 경우 중첩되지 않는다.

같은 버스를 사용하면 이전 요청이 완료되기를 기다린다.

다른 버스를 사용하면 하드웨어 적으로 동시에 실행이 가능함으로 이득이 있다.

AIO 요청이 연속적으로 이뤄지면?

AIO 작업 큐가 있다.

작업 요청을 할 때마다 시스템 호출이 이뤄지는 것이 아니다.

작업 큐의 내용을 일정 주기마다 시스템에서 동기화한다. → 잦은 입출력이 발생하더라도, 시스템 호출(Context Switching)이 줄어든다.

최근 커널들은 하드웨어 기술의 발달로

하드웨어마다 속도와 연결거리가 다르다.

BUS - 하드웨어

BUS - BUS - 하드웨어

AIO는 근접 I/O(대개의 경우 빠른 I/O)를 먼저 처리하도록 유도한다 → 응답성이 더 좋아지는 효과가 있다.

AIO는 librt 라이브러리에서 제공 gcc -lrt

AIO 함수들

입출력 → 

aio_read : 비동기화된 입력 요청, 

aio_write : 비동기화된 출력 요청, lio_listio : 비동기 I/O 리스트를 처리

확인 → 

aio_error: 비동기 입출력의 에러 및 상황 확인, 

aio_return : 비동기 입출력 작업을 동기화

집행 → 

aio_fsync : 비동기적 입출력 작업을 동기화(다 처리할 때 까지 기다림), 

aio_suspend : 비동기 입출력 처리를 기다림, 타임아웃 지정 가능(10개중 가장 먼저 끝나는 하나를 기다림)

aio_cancel : 미처리된 비동기 입출력 요청을 취소 → 이미 처리된 입출력은 취소되지 않는다

```c
struct aiocb{
	int aio_fildes; //파일 기술자
	off_t aio_offset; //I/O를 처리할 파일 오프셋 -> 비동기 처리임으로 작업이 시작될 때, 파일 기술자의 기존 파일 오프셋이 어디를 가리키고 있을 지 알 수 없음(변경되어 있을 가능성이 높음)
	volatile void *aio_buf; //버퍼 위치
	size_t aio_nbytes;  //버퍼 길이// 쓰기 : 실제 쓸 내용의 크기, 읽기 : 읽을 수 있는 버퍼의 최대 크기
	int aio_reqprio; //요청된 우선순위 0~sysconf(_SC_AIO_PRIO_DELTA_MAX) 사이의 값
	struct sigevent aio_sigevent; //발생시킬 시그널 이벤트 구조체 
	int aio_lio_opcode; //listio 함수 사용시 사용, 한 번에 여러 개의 AIO 작업을 처리, 동시에 여러개 읽기 혹은 쓰기 요청(벡터연산), 읽기/쓰기 교차 가능
	int aio_flags; //flags
}

//비동기적 읽기, 쓰기 -> 호출 성공 시 I/O 작업이 AIO 작업 큐에 등록됨을 의미, 반환 값 성공:0, 실패:-1
int aio_read(struct aiocb *aiocbp);
int aio_write(struct aiocb *aiocbp);

//비동기적 결과 확인
int aio_error(struct aiocb *aiocbp); //앞서 등록한 AIO 작업 요청의 오류 발생 혹은 완료 여부 확인
//결과값
0 : 오류없이 작업 성공
EINPORGRESS (I/O작업이 아직 완료 안됨)
ECANCELED (I/O작업이 취소됨)
EINVAL (인수(aiocbp)가 잘못됨) //sigevent를 사용하면 완료되었는지 결과를 확인할 필요가 없음

ssize_t aio_return(struct aiocb *aiocbp); //작업 결과 확인 및 작업 큐에서 제거
// aio_read 는 read의 결과값 : 성공 시 읽은 바이트 수 실패는 -1
// aio_write는 write의 결과값
// aio_fsync는 fsync의 결과값

//비동기적 동기화, fsync를 비동기적으로 실행, op: fsync의 op, 
//O_SYNC : 파일의 메타 정보 + 데이터 모두 동기화,
//O_DSYNC : 데이터만 동기화 -> 파일시스템 처리가 오래걸리는 경우에 씀
int aio_fsync(int op, struct aiocb *aiocbp);

//비동기적 I/O 요청 완료 기다리기, cblist에 있는 I/O의 완료를 기다림 -> 이 중 하나만 완료되어도 기다리기를 종료한다. timeout은 대기 최대 시간 지정
//반환값 : 성공 = 0, 실패 = -1 : errno에 에러 설정, EAGAIN : 타임 아웃으로 종료한 경우, EINTR : 시그널에 의해 중단된 경우, ENOSYS : 시스템이 지원하지 않는 경우
int aio_suspend(const struct aiocb *const cblist[], const struct timespec *timeout);

//복수의 비동기적 I/O 요청, 복수의 작업(read / write)를 한번에 요청하는 것
// mode 
// LIO_WAIT : 작업 완료 대기(블록킹) -> 요청한 I/O 처리 완료
// LIO_NOWAIT : 작업 완료 대기 안함 (넌블록킹 + 비동기 처리) -> 결과를 확인하려면 ? aio_error + aio_return의 반복? -> sigevent를 이용한 리얼타임 시그널 이벤트 처리
// aiocb 배열
// 주의 요청 처리 중에 변경하지 않도록 한다. 배열에 NULL이 있으면 무시하고 다음 항목을 진행한다. aio_lio_opcode에 따라 작동방법이 결정된다 -> LIO_READ, LIO_WRITE, LIO_NOP
// nitems -> aiocb 배열의 개수, 이 크기는 _POSIX_AIO_LISTIO_MAX 보다 작아야 한다.
// sevp -> 완료시 리얼타임 시그널 이벤트 통지/처리 방법, NULL 인 경우 무시, mode가 LIO_WAIT인 경우에도 무시한다.
int lio_listio(int mode, struct aiocb* const aiocb_list[], int nitems, struct sigevent *sevp);

```

AIO 작업 큐

큐의 크기 : _POSIX_AIO_MAX

이보다 많은 요청을 하게 되면 aio_XXXX 요청이 실패 → errno에 EAGAIN이 설정된다.

만약 요청을 등록할 수 없다면 aio_suspend로 몇몇 AIO 작업이 완료되기를 기다린다.

컴파일 시 librt를 꼭 링크해야함

# 14-2

posix_spawn 계열 함수 → Fork-Exec 연계를 대체하는 표준

왜 필요한가? 

fork 후 바로 exec를 하는 경우 → 부모 프로세스의 자원을 선택적으로 복제할 수 없음

시그널, 파일기술자 등

복제하지 않을 요소는 직접 다 처리해야함 → 코드의 복잡도 상승, 예기치 않은 문제 발생 가능성 상승

부모 프로세스의 자원을 선택적으로 복제할 수 있는 인터페이스

posix_spawn 계열 함수(POSIX 표준)으로 제안

```c
int posix_spawn(pid_t *restrict pid, const char *restrict path, const posix_spawn_file_actions_t *restrict file_actions,
								const posix_spawnattr_t *restrict attrp, char *const argv[restrict], char *const envp[restrict]);

int posix_spawnp(pid_t *restrict pid, const char *restrict file, const posix_spawn_file_actions_t *restrict file_actions,
								const posix_spawnattr_t *restrict attrp, char *const argv[restrict], char *const envp[restrict]);
```

반환 값 : int

성공 : 0

실패 : EINVAL, EACCESS 등의 값 ( >0)

→ POSIX 표준에서는 스레드 환경을 고려하여 전역변수 errno를 사용하지 않음

첫 번째 인자 : pid → Spawn이 성공하였을 때, (fork된 후) 프로세스의 PID

두 번째 인자 : path → Spawn 할 실행 파일 경로, (exec)

세번째 인자 : file_actions → Spawn 시 열거나 닫을 파일의 정보를 담은 구조체 , 기본 : 부모 프로세스가 열었던 파일은 모두 상속받음, 추가로 열어야 하거나 닫아야 하는 파일에 대한 정보를 담기 위해 사용

네번째 인자 : attrp → Spawn 시, 프로세스의 속성(프로세스 그룹, 세션 등, 시그널의 상속 및 설정 등)

다섯번째 인자 : argv → Spawn 할 실행 파일에 대한 입력 인자 목록

여섯번째 인자 : envp → Spawn 할 실행 프로세스의 환경 변수 목록

POSIX_SPAWN_FILE_ACTIONS_T

```c
//구조체의 초기화, 사용전 반드시 초기화 필요
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);
//구조체에서 연결된 메모리 사용 해제
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);
//자식 프로세스 생성시 추가로 open 할 파일 지정 -> filedes 열린 파일의 기술자 번호(열린 파일을 dup2로 복제하는 것과 같음), path, oflag, mode는 open() 함수의 인수와 같음
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *restrict file_actions, int filedes, const char*restrict path, int oflag, mode_t mode);
//자식 프로세스 생성 후 exec할 때 close 할 파일 기술자 -> 해당 파일 기술자에 close-on-exec 기능 설정
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions, int filedes);
//자식 프로세스 생성 후 파일 기술자를 복제해야 할 때 -> fork 후, exec하기 전에 dup2 하는 것과 같음
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions, int filedes, int newfiledes);
```

POSIX_SPAWNATTR_T

```c
//posix_spawnattr_t 구조체 초기화
int posix_spawnattr_init(posix_spawnattr_t *attr);
//posix_spawnattr_t 구조체에 연결된 메모리 해제
int posix_spawnattr_destroy(posix_spawnattr_t *attr);
int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);
int posix_spawnattr_getflags(const posix_spawnattr_t *restrict attr, short *restrict flags);

//flags
POSIX_SPAWN_RESETIDS -> 자식 프로세스의 EUID(Effective UID) = 부모프로세스의 RUID(Real UID)
POSIX_SPAWN_SETPGROUP - 프로세스 그룹 설정 활성화, posix_spawnattr_setpggroup으로 변경
```
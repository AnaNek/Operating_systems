#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define ACT_READER 0
#define ACT_WRITER 1
#define WAIT_READER 2
#define WAIT_WRITER 3

#define p -1
#define v 1

int* buf;

#define count 10

struct sembuf start_read[5] = {{WAIT_READER, v, IPC_CREAT | SEM_UNDO}, // инкремент счётчика ждущих читателей
		               {WAIT_WRITER, 0, IPC_CREAT | SEM_UNDO}, // проверка счётчика ждущих писателей на 0
                               {ACT_WRITER, 0, IPC_CREAT | SEM_UNDO}, // проверка, есть ли активный писатель
                               {ACT_READER, v, IPC_CREAT | SEM_UNDO}, // захват семафора активного читателя
			       {WAIT_READER, p, IPC_CREAT | SEM_UNDO}}; // декремент счётчика ждущих читателей

struct sembuf  start_write[4] = {{WAIT_WRITER, v, IPC_CREAT | SEM_UNDO}, // инкремент счётчика ждущих писателей
                                 {ACT_READER, 0, IPC_CREAT | SEM_UNDO},  // проверка, есть ли активные читате
                                 {ACT_WRITER, p, IPC_CREAT | SEM_UNDO},  // захват семафора активного писателя 
                                 {WAIT_WRITER, p, IPC_CREAT | SEM_UNDO}}; // декремент счётчика ждущих писателей

struct sembuf  stop_read[1] = {{ACT_READER, p, IPC_CREAT | SEM_UNDO}};

struct sembuf  stop_write[1] = {{ACT_WRITER, v, IPC_CREAT | SEM_UNDO}}; 

pid_t pidp[count];

void catch_sigint(int sig_num)
{
    for (int i = 0; i < count; i++)
    {
         kill(i, SIGTERM);
    }
}

void writer(int fd, int i)
{
    srand(getpid());
    while(1)
    {
        if (semop(fd, start_write, 4) == -1)
        {
           perror("semop p\n");
           exit(1);
        }

        printf("Writer[%d] value: %d\n", i, ++(*buf));

        if (semop(fd, stop_write, 1) == -1)
        {
            perror("semop v\n");
            exit(1);
        }
	sleep(rand() % 5);
    }
}

void reader(int fd, int i)
{
    srand(getpid());
    while(1)
    {
        if (semop(fd, start_read, 5) == -1)
        {
            perror("semop p\n");
            exit(1);
        }

        printf("Reader[%d] value: %d\n", i, *buf);

        if (semop(fd, stop_read, 1) == -1)
        {
            perror("semop v\n");
            exit(1);
        }
	sleep(rand() % 5);
    }
}

int main()
{
    int status;
    int buf_id;
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int fd = semget(IPC_PRIVATE, 4 , IPC_CREAT | perms);

    signal(SIGINT, catch_sigint);

    if (fd == -1)
    {
        perror("semget\n");
        exit(1);
    }

    int ctrl_actR = semctl(fd, ACT_READER, SETVAL, 0);
    int ctrl_actW = semctl(fd, ACT_WRITER, SETVAL, 1); 
    int ctrl_waitW = semctl(fd, WAIT_WRITER, SETVAL, 0);
    int ctrl_waitR = semctl(fd, WAIT_READER, SETVAL, 0);

    if ((ctrl_actR == -1) || (ctrl_actW == -1) || (ctrl_waitR == -1) || (ctrl_waitW == -1))
    {
	perror("ctrl\n");
        exit(1);
    }

    if ((buf_id = shmget(IPC_PRIVATE, 1 * sizeof(int), IPC_CREAT | perms)) == -1)
    {
	perror("shmget\n");
	exit( 1 );
    }

    buf = (int *) shmat(buf_id, 0, 0);
    if (buf == (int *)-1)
    {
        perror("shmat\n");
        exit(1);
    }

    *buf = 0;

    for (int i = 0; i < count; i++)
    {
        if ((pidp[i] = fork()) == -1)
        {
            perror("Couldn't fork.");
            exit(1);
        }
        else if (pidp[i] == 0)
        {
            if (i % 2)
            {
	        writer(fd, i);
            }
            else
            {
                reader(fd, i);
            }
            return 0;
        }
    }

    while (wait(NULL) != -1);

    if (shmdt(buf) == -1) 
    {
        perror("shmdt\n");
	exit(1);
    }
    return 0;
}

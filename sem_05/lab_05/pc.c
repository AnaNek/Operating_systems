#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define sb 0
#define se 1
#define sf 2

#define p -1
#define v 1

int n = 20;

#define count 6
// nowait - не переход в сост ожид если не может захват
// undo процесс завершился, сист должна отменить все операции соверш на семафоре

struct sembuf producer_p[2] = {{se, p, IPC_CREAT | SEM_UNDO}, {sb, p, IPC_CREAT | SEM_UNDO}};
struct sembuf producer_v[2]  =  {{sb, v, IPC_CREAT | SEM_UNDO}, {sf, v, IPC_CREAT | SEM_UNDO}};
struct sembuf consumer_p[2] = {{sf, p, IPC_CREAT | SEM_UNDO}, {sb, p, IPC_CREAT | SEM_UNDO}};
struct sembuf consumer_v[2]  =  {{sb, v, IPC_CREAT | SEM_UNDO}, {se, v, IPC_CREAT | SEM_UNDO}};

pid_t pidp[count];

void catch_sigint(int sig_num)
{
    for (int i = 0; i < count; i++)
    {
         kill(i, SIGTERM);
    }
}

void producer(int *buf, int *prod, int fd, int i)
{
    while(1)
    {
	int val;
        if (semop(fd, producer_p, 2) == -1)
        {
           perror("semop p\n");
           exit(1);
        }

        val = *(buf + *prod);
	val++;
        if (++(*prod) == n)
	{
	    *prod = 0;
	}
	*(buf + *prod) = val;
        printf("Producer[%d] value: %d\n", i, val);

        if (semop(fd, producer_v, 2) == -1)
        {
            perror("semop v\n");
            exit(1);
        }
	sleep(rand()%3);
    }
}

void consumer(int *buf, int *cons, int fd, int i)
{
    while(1)
    {
        int val;
        if (semop(fd, consumer_p, 2) == -1)
        {
            perror("semop p\n");
            exit(1);
        }

	if (++(*cons) == n)
	{
	    *cons = 0;
	}
        val = *(buf + *cons);
        printf("Consumer[%d] value: %d\n", i, val);
        if (semop(fd, consumer_v, 2) == -1)
        {
            perror("semop v\n");
            exit(1);
        }
	sleep(rand()%5);
    }
}

int main()
{
    int status;
    int buf_id;
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int *buf;
    int *cons;
    int *prods;

    int fd = semget(IPC_PRIVATE, 3 , IPC_CREAT | perms);

    signal(SIGINT, catch_sigint);

    if (fd == -1)
    {
        perror("semget\n");
        exit(1);
    }

    if ((semctl(fd, se, SETVAL, (n)) == -1) || (semctl(fd, sb, SETVAL, 1) == -1) || semctl(fd, sf, SETVAL, 0) == -1)
    {
	perror("ctl\n");
        exit(1);
    }

    if ((buf_id = shmget(IPC_PRIVATE, (n + 2) * sizeof(int), IPC_CREAT | perms)) == -1)
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

    prods = buf;
    cons = buf + 1;
    buf += 2;

    *buf = 0;
    *prods = 0;
    *cons = 0;

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
	        producer(buf, prods, fd, i);
            }
            else
            {
                consumer(buf, cons, fd, i);
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

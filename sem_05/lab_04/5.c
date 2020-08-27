#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int catched = 0;

void catch_sigint(int sig_num)
{
    catched = 1;
}

int main()
{
    signal(SIGINT, catch_sigint);

    pid_t child[2];
    int status;
    char msg[2][16] = {"msg1 ", "msg2"};
    char buf[16];
    int mypipe[2];

    if (pipe(mypipe) == -1)
    {
        perror("Couldn't pipe.");
        exit(1);
    }

    for (int i = 0; i < 2; i++)
    {
        if ((child[i] = fork()) == -1)
        {
            perror("Couldn't fork.");
            exit(1);
        }
        else if (child[i] == 0)
        {
            if (!catched)
                sleep(4);

            if (catched)
            {
                close(mypipe[0]);
                write(mypipe[1], msg[i], strlen(msg[i]) + i);
            }
            printf("Child%d: pid=%d, group=%d, parent=%d\n\n", (i + 1), getpid(), getgid(), getppid());
            return 0;
        }
        else
        {
            printf("parent: pid = %d, child%d = %d, gid = %d\n",getpid(), (i + 1), child[i], getgid());
            wait(&status);
            if (WIFEXITED(status))
                printf("Child%d exited with code %d\n", (i + 1), WEXITSTATUS(status));
            else
                printf("Child%d terminated abnormally\n", (i + 1));

            if (catched && (i == 1))
            {
                close(mypipe[1]);
                read(mypipe[0], buf, sizeof(buf));
                printf("Catch msg - %s\n", buf);
            }
        }
    }

    return 0;
}

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int main()
{
    pid_t child[2];

    for (int i = 0; i < 2; i++)
    {
        if ((child[i] = fork()) == -1)
        {
            perror("Couldn't fork.");
            exit(1);
        }
        else if (child[i] == 0)
        {
            printf("Child%d: pid=%d, group=%d, parent=%d\n\n", (i + 1), getpid(), getgid(), getppid());
            sleep(2);
            printf("Child%d: pid=%d, group=%d, parent=%d\n\n", (i + 1), getpid(), getgid(), getppid());
            return 0;
        }
        else
        {
            printf("parent: pid = %d, child%d = %d, gid = %d\n",getpid(), (i + 1), child[i], getgid());
        }
    }

    return 0;
}

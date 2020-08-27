#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	pid_t child[2];
    int status;

    for (int i = 0; i < 2; i++)
    {
        if ((child[i] = fork()) == -1)
        {
            perror("Couldn't fork.");
            exit(1);
        }
        else if (child[i] == 0)
        {
			if (execl("/bin/ls","ls","-l", NULL) == -1)
			{
				perror("Couldn't exec.\n");
				exit(1);
			}
            printf("Child%d: pid=%d, group=%d, parent=%d\n\n", (i + 1), getpid(), getgid(), getppid());
            return 0;
        }
        else
        {
            printf("parent: pid = %d, child%d = %d, gid = %d\n",getpid(), (i + 1), child[i], getgid());
            wait(&status);
            printf("Child%d has finished: PID = %d\n",(i + 1), child[i]);
            if (WIFEXITED(status))
                printf("Child%d exited with code %d\n", (i + 1), WEXITSTATUS(status));
            else
				printf("Child%d terminated abnormally\n", (i + 1));
        }
    }

	return 0;
}

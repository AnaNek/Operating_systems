#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCK_NAME "socket.soc"
#define BUF_SIZE 256

int main(int argc, char ** argv)
{
    int   sock;
    char buf[BUF_SIZE];
    struct sockaddr srvr_name;

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("socket() failed");
        return EXIT_FAILURE;
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);
    sprintf(buf, "pid %d", getpid());

    printf("Client's msg: %s\n", buf);
    sendto(sock, buf, strlen(buf), 0, &srvr_name,
    strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family));

    close(sock);
    return EXIT_SUCCESS;
}

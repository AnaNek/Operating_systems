#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define PORT 3425
#define BUF_SIZE 256
#define COUNT 5
#define SOCK_ADDR "localhost"

int main(int argc, char ** argv)
{
    int sock;
    struct sockaddr_in serv_addr;
    struct hostent *host;
    char buf[BUF_SIZE];
    char message[BUF_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
      printf("socket() failed: %d", errno);
      return EXIT_FAILURE;
    }

    host = gethostbyname(SOCK_ADDR);
    if (!host)
    {
        perror("gethostbyname() failed: ");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr*) host->h_addr_list[0]);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("connect() failed: %d", errno);
      return EXIT_FAILURE;
    }

    srand(time(NULL));

    for (int i = 0; i < COUNT; i++)
    {
        memset(message, 0, BUF_SIZE);
        sprintf(message, "message[%d]\n", i);

        if (send(sock, message, sizeof(message), 0) < 0)
        {
            perror("send() failed:");
            return EXIT_FAILURE;
        }

        recv(sock, buf, sizeof(message), 0);

        printf("Server got %s\n", buf);

        sleep(1 + rand() % 5);
    }

    close(sock);

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 256
#define PORT 3425
#define NUMBER_OF_CLIENTS 5

void receive(int *clients, int n, fd_set *set)
{
    char buf[BUF_SIZE];
    int bytes;

    for (int i = 0; i < n; i++)
    {
        if (FD_ISSET(clients[i], set))
        {
            // Поступили данные от клиента, читаем их
            bytes = recv(clients[i], buf, BUF_SIZE, 0);

            if (bytes <= 0)
            {
                // Соединение разорвано, удаляем сокет из множества
                printf("Client[%d] disconnected\n", i);
                close(clients[i]);
                clients[i] = 0;
            }
            else
            {
                // Отправляем данные обратно клиенту
                buf[bytes] = 0;
                printf("Client[%d] sent %s\n", i, buf);
                send(clients[i], buf, bytes, 0);
            }
        }
    }
}

int main(int argc, char ** argv)
{
     int sock;
     int new_sock;
     struct sockaddr_in serv_addr;
     fd_set set;
     int clients[NUMBER_OF_CLIENTS] = {0};
     int mx;
     int flag = 1;

     sock = socket(AF_INET, SOCK_STREAM, 0);
     if (socket < 0)
     {
       printf("socket() failed: %d\n", errno);
       return EXIT_FAILURE;
     }

     fcntl(sock, F_SETFL, O_NONBLOCK);

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(PORT);

     if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
     {
       printf("bind() failed: %d\n", errno);
       return EXIT_FAILURE;
     }

     if (listen(sock, 6) < 0)
     {
         printf("listen() failed: %d\n", errno);
         return EXIT_FAILURE;
     }

     printf("waiting...\n");

     while(1)
     {
         // Заполняем множество сокетов
         FD_ZERO(&set);
         FD_SET(sock, &set);
         mx = sock;

         for (int i = 0; i < NUMBER_OF_CLIENTS; i++)
         {
             if (clients[i])
             {
                 FD_SET(clients[i], &set);
             }
             mx = (mx > clients[i]) ? mx : clients[i];
         }

         // Ждём события в одном из сокетов
         if (select(mx + 1, &set, NULL, NULL, NULL) <= 0)
         {
             perror("select");
             exit(1);
         }

         // Определяем тип события и выполняем соответствующие действия
         if (FD_ISSET(sock, &set))
         {
             // Поступил новый запрос на соединение, используем accept
             new_sock = accept(sock, NULL, NULL);

             if (new_sock < 0)
             {
                 perror("accept");
                 exit(1);
             }

             fcntl(new_sock, F_SETFL, O_NONBLOCK);

             flag = 1;
             for (int i = 0; i < NUMBER_OF_CLIENTS && flag; i++)
             {
                 if (!clients[i])
                 {
                     clients[i] = new_sock;
                     printf("Added as client №%d\n", i);
                     flag = 0;
                 }
             }
         }

         receive(clients, NUMBER_OF_CLIENTS, &set);
     }
     return EXIT_SUCCESS;
}

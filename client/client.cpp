#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

#define BUFSIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFSIZE];
    std::string input;

    if (argc < 3)
    {
        fprintf(stderr, "usage %s localhost 1234\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1)
    {
        bzero(buffer, BUFSIZE);
        n = read(sockfd, buffer, BUFSIZE - 1);
        if (n < 0)
            error("ERROR reading from socket");
        std::cout << buffer;
        bzero(buffer, BUFSIZE);
        std::cin.getline(buffer, 1024, '\n');
        // std::cout << strlen(buffer) << "\n";
        if (strlen(buffer) == 0)
        {
            // 这里是为了处理直接按回车的问题
            buffer[0] = ' ';
            buffer[1] = '\0';
        }
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");
    }
    close(sockfd);
    return 0;
}
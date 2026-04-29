#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define PORT 2728
#define BACKLOG 10
#define SIZE 1024


void getFileURL(char *rout, char *fileURL)
{
    char *question = strrchr(rout, '?');
    if (question)
    {
        *question = '\0';
    }
    if (rout[strlen(rout) - 1] == '/')
    {
        strcat(rout, "index.html");
    }
    strcpy(fileURL, "myfolder");
    strcat(fileURL, rout);

    const char *dot = strrchr(fileURL, '.');
    if (!dot || dot == fileURL)
    {
        strcat(fileURL, "index.html");
    }
}

void getMimeType(char *file, char *mime)
{
    const char *dot = strrchr(file, '.');
    if (dot == NULL) strcpy(mime, "text/html");
    else if (strcmp(dot, ".html") == 0) strcpy(mime, "text/html");
    else if (strcmp(dot, ".css") == 0) strcpy(mime, "text/css");
    else if (strcmp(dot, ".js") == 0) strcpy(mime, "text/js");
    else if (strcmp(dot, ".png") == 0) strcpy(mime, "text/png");
    else if (strcmp(dot, ".jpg") == 0) strcpy(mime, "text/jpg");
    else if (strcmp(dot, ".gif") == 0) strcpy(mime, "text/gif");
    else if (strcmp(dot, ".c") == 0) strcpy(mime, "text/c");
    else strcpy(mime, "text/html");
}

void getTimeString(char *buf)
{
    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", gmt);
}


int main()
{
    int serverSocket;
    struct sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error: The server is not bound to the address.\n");
        return 1;
    }
    if (listen(serverSocket, BACKLOG) < 0)
    {
        printf("Error: The server is not listening.\n");
        return 1;
    }

    char hostBuffer[NI_MAXHOST], serviceBuffer[NI_MAXSERV];
    int error = getnameinfo((struct sockaddr *)&serverAddress, sizeof(serverAddress), hostBuffer, sizeof(hostBuffer), serviceBuffer, sizeof(serviceBuffer), 0);

    if (error != 0)
    {
        printf("Error: %s\n", gai_strerror(error));
    }
    printf("\n Server is listening on http://%s:%s/\n\n", hostBuffer, serviceBuffer);

    while (1)
    {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) continue;

        char *request = (char *)malloc(SIZE * sizeof(char));
        read(clientSocket, request, SIZE);

        char method[10], rout[100];
        sscanf(request, "%s %s", method, rout);
        printf("%s %s", method, rout);
        free(request);

        char fileURL[100];
        getFileURL(rout, fileURL);

        FILE *file = fopen(fileURL, "r");

        if (!file)
        {
            const char response[] = "HTTP/1.1 404 Not Found\r\n\n";
            send(clientSocket, response, strlen(response), 0);
            close(clientSocket);
            printf(" [404 Not Found]\n");
            continue;
        }

        char mimeType[32];
        getMimeType(fileURL, mimeType);
        char timeBuffer[100];
        getTimeString(timeBuffer);

        char resHeader[SIZE];
        sprintf(resHeader, "HTTP/1.1 200 OK\r\nDate: %s\r\nContent-Type: %s\r\n\n", timeBuffer, mimeType);
        int headerSize = strlen(resHeader);

        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *resBuffer = (char *)malloc(fsize + headerSize);
        strcpy(resBuffer, resHeader);
        char *fileBuffer = resBuffer + headerSize;
        fread(fileBuffer, fsize, 1, file);

        send(clientSocket, resBuffer, fsize + headerSize, 0);
        printf(" %s\n", mimeType);

        free(resBuffer);
        fclose(file);
        close(clientSocket);
    }

    return 0;
}
#include <asm-generic/socket.h>
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

int main() {
  int serverSocket;
  int clientSocket;
  char *request;

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

  if(error != 0){
    printf("Error: %s\n", gai_strerror(error));
  }

  printf("\n Server is listening on http://%s:%s/\n\n", hostBuffer, serviceBuffer);
  
  request = (char *)malloc(SIZE * sizeof(char));
  char method[10], rout[100];

  clientSocket = accept(serverSocket, NULL, NULL);
  read(clientSocket, request, SIZE);

  sscanf(request,"%s %s",method, rout );
  printf("%s %s",method, rout );
  free(request);


  return 0;
}

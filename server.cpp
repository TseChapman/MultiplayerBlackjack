#include <iostream>
#include <cstring>
#include <string>
#include <sys/types.h> // socket, bind
#include <sys/socket.h> // socket, bind, listen, inet_ntoa
#include <sys/time.h>
#include <netinet/in.h> // htonl, htons, inet_ntoa
#include <arpa/inet.h> // inet_ntoa
#include <netdb.h> // gethostbyname
#include <unistd.h> // read, write, close
#include <strings.h> // bzero
#include <netinet/tcp.h> // SO_REUSEADDR
#include <sys/uio.h> // writev

using namespace std;

struct thread_data {
  int sd;
};

static void * serverThreadFunc(void *arg) {
  // Define server function variables
  struct thread_data *data = (struct thread_data*)arg; // data object
  // TODO: Find ways to get json stringtify input
  const int BUFSIZE = 1500;
  char databuf[BUFSIZE]; // where nbufs * bufsize = 1500

  // Save the start time
  struct timeval start;
  gettimeofday(&start, NULL);

  // TODO: Read Client Action code
  // read username from client
  // While loop
  // write username return status back to client
  // If return status is false, read new username from client
  // if return status is true, save username, sd in an object in a list, break the loop.

  // While loop
  // read action code from client
  // write server action code status (yes/no)
  /*
  use cases:
  1.
  2.
  3.
  */
  // Write server output

  // Read client input (TODO: Transform input into object/json)
  //read(data->sd, databuf, BUFSIZE - nRead)

  // Calculate receiving time
  struct timeval stop;
  gettimeofday(&stop, NULL);
  suseconds_t receiving_time = (
    (stop.tv_sec * 1000000 + stop.tv_usec) -
    (start.tv_sec * 1000000 + start.tv_usec)
  );

  // Return the game information json object to client
  //write(data->sd, &count, sizeof(count));
  cout << "data-receiving time = " << receiving_time << " usec" << endl;
  close(data->sd);
}

int main (int argc, char* argv[]) {
  // Validate parameters
  if (argc != 2) { return -1; }
  char* port;
  try {
    port = argv[1];
  } catch (...) {
    cerr << "Error happened getting parameter" << endl;
    return -1;
  }

  // Get address information
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, port, &hints, &res);

  try {
    // Create Server
    int serverSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    const int yes = 1;
    const int maxNumClient = 100;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes));
    bind(serverSd, res->ai_addr, res->ai_addrlen);
    listen(serverSd, maxNumClient);

    // Accept an incomming connection
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    while (1) {
      // Accept connection
      int newSd = accept(serverSd, (struct sockaddr *)&clientAddr, &clientAddrSize);

      // Start new thread and perform function serverThreadFunc
      pthread_t newThread;
      struct thread_data *data = new thread_data;
      data->sd = newSd;
      int clientAction = pthread_create(&newThread, NULL, serverThreadFunc, (void*)data);
    }
  } catch (...) {
    cerr << "Error happened in creating server and accepting connections" << endl;
    return -1;
  }
}
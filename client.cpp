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

int main(int argc, char* argv[]) {
  // Validate parameters
  if (argc != 6) { return -1; }
  char* serverName;
  char* serverPort;
  char* username;
  int nbufs;
  int bufsize;
  try {
    serverName = argv[1];
    serverPort = argv[2];
    username = argv[3];
    nbufs = stoi(argv[4]);
    bufsize = stoi(argv[5]);
  } catch (...) {
    cerr << "Error happened getting parameter" << endl;
    return -1;
  }

  // Load address structs with getaddrinfo
  struct addrinfo hints, *servInfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  getaddrinfo(serverName, serverPort, &hints, &servInfo);

  try {
    // Make a socket, bind it, and listen on it
    int clientSd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);

    int status = connect(clientSd, servInfo->ai_addr, servInfo->ai_addrlen);
    if (status < 0) {
      cerr << "Failed to connect to the server" << endl;
      close(clientSd);
      return -1;
    }

    char databuf[nbufs][bufsize]; // where nbufs * bufsize = 1500

    // Save start time
    struct timeval start;
    gettimeofday(&start, NULL);

    // write username to server
    // While loop
    // read server return status
    // If return status is false, ask for new username and write to server again
    // if return status is true, username is set, break the loop.

    // While loop
    // Get user input command
    // write action code to server
    // read server action code status
    /*
    use cases:
    1.
    2.
    3.
    */
    // Read server output
    // Display on screen


    // Handle write to server by type
    /*
    if (type == 1) {
      for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < nbufs; j++) {
          int x = write(clientSd, databuf[j], bufsize);
          if (x < 0) {
            cerr << "Problem with multiple writes" << endl;
          }
        }
      }
    }
    else if (type == 2) {
    for (int i = 0; i < iterations; i++) {
        struct iovec vector[nbufs];
        for (int j = 0; j < nbufs; j++) {
          vector[j].iov_base = databuf[j];
          vector[j].iov_len = bufsize;
        }
        writev(clientSd, vector, nbufs);
      }
    }
    else if (type == 3) {
      for (int i = 0; i < iterations; i++) {
        write(clientSd, databuf, nbufs * bufsize);
      }
    }
    */
    // Save transmission time
    struct timeval lap;
    gettimeofday(&lap, NULL);
    suseconds_t transmission_time = (
      (lap.tv_sec * 1000000 + lap.tv_usec) -
      (start.tv_sec * 1000000 + start.tv_usec)
    );

    // Use read to receive a response from the server
    /*
    int count = 0;
    read(clientSd, &count, sizeof(count));
    */
    // Save round trip time
    struct timeval stop;
    gettimeofday(&stop, NULL);
    suseconds_t round_trip_time = (
      (stop.tv_sec * 1000000 + stop.tv_usec) -
      (start.tv_sec * 1000000 + start.tv_usec)
    );

    // Display output
    cout << "Test 1: data-transmission time = " << transmission_time << " usec, round-trip time = " << round_trip_time
  << " usec, #reads = " << count << endl;
    close(clientSd);
  } catch (...) {
    cerr << "Error happened when connecting with server" << endl;
    return -1;
  }

}
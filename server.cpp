#include <arpa/inet.h> // inet_ntoa
#include <cstring>
#include <errno.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <netinet/in.h> // htonl, htons, inet_ntoa
#include <netinet/tcp.h> // SO_REUSEADDR
#include <netdb.h> // gethostbyname
#include <unistd.h> // read, write, close
#include <unordered_map>
#include <sstream>
#include <string>
#include <strings.h> // bzero
#include <sys/types.h> // socket, bind
#include <sys/socket.h> // socket, bind, listen, inet_ntoa
#include <sys/time.h>
#include <sys/uio.h> // writev
#include <vector>
#include "Game.h"

using namespace std;

extern int errno;

const int MAX_CON = 10;

vector<Game> lobbies;
int lobby_id = 0;

struct Player {
  string player_id;
  string username;
  int status; // 0 for not active, 1 for active
  string lobby_id; // empty if not in lobby
};

unordered_map<string, Player> players;
int player_id;
pthread_mutex_t players_mutex;

void printDebugStatement(bool isDebug, string statement) {
  if (isDebug) {
    cout << statement << endl;
  }
}

struct thread_data {
  int thread_id;
  int sd;
  int isDebug;
};

string parseRequestInfo(int sd) {
  // Parse request header from client socket
  string requestLine = "";
  char prev = 0;
  while (true) {
    char curr = 0;
    read(sd, &curr, 1);
    if (curr == '\n' || curr == '\r') {
      if (curr == '\n' && prev == '\r') { break; }
    }
    else {
      requestLine += curr;
    }
    prev = curr;
  }
  return requestLine;
}

struct requestData {
  string action;
  int ack;
  string player_id;
  vector<string> information;
};

bool isLobbyNameCreated(string lobby_name) {
  for (int i = 0; i < lobbies.size(); i++) {
    if (lobbies.at(i).lobby_name == lobby_name) {
      return true;
    }
  }
  return false;
}

bool isLobbyIDCreated(string lobby_id) {
  for (int i = 0; i < lobbies.size(); i++) {
    if (lobbies.at(i).lobby_id == lobby_id) {
      return true;
    }
  }
  return false;
}

string processRequestData(const requestData& request) {
  string response = "";
  // Determine the action
  // Map of commands
  unordered_map<string, int> command_map = {
    {"EXIT", 1},
    {"CREATE_GAME", 2},
    {"EXIT_GAME", 3},
    {"LIST", 4},
    {"CREATE_USER", 5},
    {"JOIN", 6}
  };

  int command;
  try {
    command = command_map[request.action];
  } catch (...) {
    cerr << "Command not Found" << endl;
    // TODO: Finalize Invalid request
    response = "NO\r\nAck:" + to_string(request.ack) +"\r\nPlayer ID:" + request.player_id + "\r\n\r\n";
  }

  switch (command) {
    case 1: {
      // Exit the game
      pthread_mutex_lock(&players_mutex);
      // Step 1: Exit lobby if any
      try {
        if (!players[request.player_id].lobby_id.empty()) {
          // Exit lobby
          if (isLobbyIDCreated(players[request.player_id].lobby_id)) {
            for (int i = 0; i < lobbies.size(); i++) {
              if (lobbies.at(i).lobby_id == players[request.player_id].lobby_id) {
                int status = lobbies.at(i).exit(request.player_id);
                if (status == 0) {
                  // TODO: fail to exit a lobby
                }
                break;
              }
            }
          }
        }
      } catch (...) {

      }
      // Step 2: deactive username
      try {
        players[request.player_id].status = 0;
      } catch (...) {

      }
      pthread_mutex_unlock(&players_mutex);
      // TODO: Write response
    } break;
    case 2: {
      try {
        pthread_mutex_lock(&players_mutex);
        // Step 1: Search list of lobby
        if (lobbies.empty()) {
          // Lobbies is empty
          string lobbyID = to_string(lobby_id);
          Game newGame(lobbyID, request.information.at(0));
          lobbies.push_back(newGame);
          // Step 5: Write to client
          response = "OK\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:CREAT_GAME\r\nLobby ID:" + to_string(lobby_id) + "\r\n\r\n";
          pthread_mutex_unlock(&players_mutex);
          break;
        }
        // Step 2: Check if non of the lobby has the same name
        if (!isLobbyNameCreated(request.information.at(0))) {
          // Step 3: Add the lobby name to the list
          // Step 4: Generate lobby id
          string lobbyID = to_string(lobby_id);
          Game newGame(lobbyID, request.information.at(0));
          lobbies.push_back(newGame);
          // Step 5: Write to client
          response = "OK\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:CREAT_GAME\r\nLobby ID:" + to_string(lobby_id) + "\r\n\r\n";
          pthread_mutex_unlock(&players_mutex);
          break;
        } else {
          response = "NO\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:CREAT_GAME\r\n\r\n";
          pthread_mutex_unlock(&players_mutex);
          break;
        }
      } catch (...) {
        response = "NO\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:CREAT_GAME\r\n\r\n";
        break;
      }
    }
    case 3: {
      pthread_mutex_lock(&players_mutex);
      try {
        // Step 1: Search for lobby with lobby_id
        for (Game g : lobbies) {
          if (g.lobby_id == request.information.at(0)) {
            // Step 2: Check if player_id in lobby and remove player from lobby
            g.exit(request.player_id);
          }
        }
        // Step 3: Reset player's lobby id
        players[request.player_id].lobby_id = "";
        response = "OK\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:EXIT_GAME\r\nLobby ID:" + request.information.at(0) + "\r\n\r\n";
        pthread_mutex_unlock(&players_mutex);
      } catch (...) {
        response = "NO\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:EXIT_GAME\r\nLobby ID:" + request.information.at(0) + "\r\n\r\n";
        pthread_mutex_unlock(&players_mutex);
      }
    } break;
    case 4: {
      pthread_mutex_lock(&players_mutex);
      response = "OK\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:LIST\r\nNumber of Lobby:" + to_string(lobbies.size()) + "\r\n";
      // Step 1: Get a list of lobby
      for (Game g : lobbies) {
        // Step 2: Format response
        //"OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:LIST\r\nNumber of Lobby:x\r\nLobby ID:lobby_id\r\nLobby Name:lobby_name\r\n\r\n"
        response += "Lobby ID:" + g.lobby_id + "\r\nLobby Name:" + g.lobby_name + "\r\n";
      }
      response += "\r\n";
      pthread_mutex_unlock(&players_mutex);
    } break;
    case 5: {
      pthread_mutex_lock(&players_mutex);
      //OK\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n
      Player newPlayer;
      bool isUserNameTaken = false;
      // Step 1: Get the list of player
      for (auto p : players) {
        // Step 2: Check if the username is taken
        if (p.second.player_id == request.player_id) {
          isUserNameTaken = true;
          response = "NO\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:" + request.player_id + "\r\nAction:CREATE_USER\r\nLobby ID:" + request.information.at(0) + "\r\n\r\n";
          pthread_mutex_unlock(&players_mutex);
          break;
        }
      }
      // Step 3: Generate player with username and new unique player id
      if (!isUserNameTaken) {
        newPlayer.player_id = player_id;
        newPlayer.username = request.information.at(0);
        newPlayer.status = 1; // Set player active
        newPlayer.lobby_id = "";

        // Step 4: Add the player to the list
        players.insert({to_string(player_id), newPlayer});
        // Step 5: Format response
        response = "OK\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:"+ request.player_id + "\r\nAction:CREATE_USER\r\n\r\n";
        pthread_mutex_unlock(&players_mutex);
      }
    } break;
    case 6: {
      pthread_mutex_lock(&players_mutex);
      // Step 1: Check if the player already joined a lobby
      if (players.at(request.player_id).lobby_id != "") {
        // E.g. OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:JOIN\r\nLobby ID:lobby_id\r\n\r\n"
        response = "NO\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:"+ request.player_id + "\r\nAction:JOIN\r\nLobby ID:" + request.information.at(0) + "\r\n\r\n";
        pthread_mutex_unlock(&players_mutex);
        break;
      }
      // Step 2: Get a list of lobby
      for (Game g : lobbies) {
        if (g.lobby_id == request.information.at(0)) {
          // Step 3: Check if the lobby is full
          if (g.isFull()) {
            response = "NO\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:"+ request.player_id + "\r\nAction:JOIN\r\nLobby ID:" + request.information.at(0) + "\r\n\r\n";
            break;
          }
          // Step 4: Add player to lobby
          g.join(request.player_id);
          // Step 5: Fromate response
          response = "OK\r\nAck:" + to_string(request.ack) + "\r\nPlayer ID:"+ request.player_id + "\r\nAction:JOIN\r\nLobby ID:" + request.information.at(0) + "\r\n\r\n";
          break;
        }
      }
      pthread_mutex_unlock(&players_mutex);
    } break;
  }
  return response;
}

void *processRequest(void *arg) {
  struct thread_data *data = (struct thread_data*)arg; // data object
  // Process the request
  string response = "";
  requestData request;
  int iterations = 0;
  while (true) {
    // Retreive the request line
    string requestLine = parseRequestInfo(data->sd);
    if (requestLine == "") {
      cerr << "Incorrect request line format" << endl;
      break;
    }

    printDebugStatement(data->isDebug, "Response Header:\n" + requestLine);
    // Get the request action
    if (iterations == 0) {
      // Split the action by space if any
      vector<string> inputs;
      stringstream ss(requestLine);
      string str;

      while (getline(ss, str, ' ')) {
        inputs.push_back(str);
      }

      // Determine the action
      // Map of commands
      unordered_map<string, int> command_map = {
        {"EXIT", 1},
        {"CREATE_GAME", 2},
        {"EXIT_GAME", 3},
        {"LIST", 4},
        {"CREATE_USER", 5},
        {"JOIN", 6}
      };

      int command;
      try {
        command = command_map[inputs.at(0)];
      } catch (...) {
        cerr << "Command not Found" << endl;
        // TODO: Finalize Invalid request
        response = "NO\r\n";
      }

      switch (command) {
        case 2:
        case 3:
        case 5:
          request.information.push_back(inputs.at(1));
        default:
          request.action = inputs.at(0);
      }
    }

    // Get Ack
    else if (iterations == 1) {
        if (requestLine.substr(0, 5) == "Ack:") {
        int ack_recv = atoi(requestLine.substr(5, requestLine.length()).c_str());
        request.ack = ack_recv;
      }
    }

    // Get Player ID
    else if (iterations == 2) {
      if (requestLine.substr(0, 10) == "Player ID:") {
        string player_id_recv = requestLine.substr(10, requestLine.length()).c_str();
        request.player_id = player_id_recv;
      }
    }
    else {
      request.information.push_back(requestLine);
    }
    iterations++;
    if (iterations > 100) { break; }
  }

  // Get the file content based on the requested file
  response = processRequestData(request);
  printDebugStatement(data->isDebug, "Complete Response: " + response);

  // Write the file and request status back to the client
  write(data->sd, &response[0], response.size());
  close(data->sd);
}

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

int main(int argc, char* argv[]) {
  // Must input 1 and only 1 parameter
  if (argc != 3) {
    cerr << "Invalid number of argument. The program takes 2 arguments: server port number, 0 or 1 for doing debug or not." << endl;
    return -1;
  }

  // Get the port from argument
  char* port;
  int isDebug;
  try {
    port = argv[1];
    isDebug = stoi(argv[2]);
  } catch (...) {
    cerr << "Error happens when parsing argument." << endl;
    return -1;
  }

  cout << "Get address information" << endl;

  // Get address information
  struct addrinfo hints, *servInfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int addressInfoStatus = getaddrinfo(NULL, port, &hints, &servInfo);
  if (addressInfoStatus != 0) {
    cerr << "Unable to connect" << endl;
    return -1;
  }

  cout << "Starting server" << endl;

  int serverSd;
  try {
    // Create Server
    serverSd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
    const int yes = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes));
    bind(serverSd, servInfo->ai_addr, servInfo->ai_addrlen);
    listen(serverSd, MAX_CON);
  } catch (...) {
    cerr << "Error happens in creating server." << endl;
    return -1;
  }

  cout << "Start accepting client connection" << endl;

  try {
    int iterations = 1;
    while (1) {
      // Accept an incomming connection
      struct sockaddr_storage clientAddr;
      socklen_t clientAddrSize = sizeof(clientAddr);
      // Accept connection
      int newSd = accept(serverSd, (struct sockaddr *)&clientAddr, &clientAddrSize);

      if (newSd == -1) {
        cerr << "Unable to connect to client." << endl;
        continue;
      }

      // Start new thread and perform function processRequest
      pthread_t newThread;
      struct thread_data *data = new thread_data;
      data->thread_id = iterations;
      data->sd = newSd;
      data->isDebug = isDebug;
      iterations++;

      cout << "Creating new thread with thread id: " + to_string(iterations) << endl;

      int threadRes = pthread_create(&newThread, NULL, processRequest, (void*)data);
    }
  } catch (...) {
    cerr << "Error happens in accepting client connection and creating threads." << endl;
    return -1;
  }
}
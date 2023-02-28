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
#include <unordered_map>
#include <strings.h> // bzero
#include <netinet/tcp.h> // SO_REUSEADDR
#include <sys/uio.h> // writev
#include <sstream>
#include <vector>
/*
#include <windows.h>//different header file in linux
#include <future>
*/
using namespace std;
/*
// setTimeOut function
// Source: https://stackoverflow.com/questions/2485058/equivalent-to-window-settimeout-for-c
template <typename... ParamTypes>
void setTimeOut(int milliseconds,std::function<void(ParamTypes...)> func,ParamTypes... parames)
{
  std::async(std::launch::async,[=]()
  {
      Sleep(milliseconds);
      func(parames...);
  });
};
*/
void printDebugStatement(bool isDebug, string statement) {
  if (isDebug) {
    cout << statement << endl;
  }
}

string parseResponseInfo(int socket) {
  string responseLine = ""; // output string
  char prev = 0;
  // Check each character of the response line
  while (true) {
    char curr = 0;
    read(socket, &curr, 1);
    if (curr == '\n' || curr == '\r') {
      if (curr == '\n' && prev == '\r') { break; }
    }
    else {
      responseLine += curr;
    }
    prev = curr;
  }
  return responseLine; // return the response line
}

struct ActionResponse {
  int status;
  string player_id;
  string action;
  vector<string> information;
};

string formatRequest(string inputAction, const int& ack, const string& player_id) {
  // Get strings that is splitted by space
  vector<string> inputs;
  stringstream ss(inputAction);
  string str;

  while (getline(ss, str, ' ')) {
    inputs.push_back(str);
  }

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
    return "";
  }

  // Format request
  string res = "";
  switch (command) {
    case 1:
      // E.g. "EXIT\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n"
      res = inputs.at(0) + "\r\n" + "Ack:" + to_string(ack) + "\r\n" + "Player ID:" + player_id + "\r\n\r\n";
      // E.g. Response: "OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:EXIT\r\n\r\n"
      break;
    case 2:
      try {
        // E.g. "CREATE_GAME lobby_name\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n"
        res = inputs.at(0) + " " + inputs.at(1) + "\r\n" + "Ack:" + to_string(ack) + "\r\n" + "Player ID:" + player_id + "\r\n\r\n";
        // E.g. Response: "OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:CREAT_GAME\r\nLobby ID:lobby_id\r\n\r\n"
      } catch (...) {
        cerr << "Invalid number of parameters for action: " + inputs.at(0) << endl;
      }
      break;
    case 3:
      try {
        // E.g. "EXIT_GAME lobby_id\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n"
        res = inputs.at(0) + " " + inputs.at(1) + "\r\nAck:" + to_string(ack) + "\r\nPlayer ID:" + player_id + "\r\n\r\n";
        // E.g. Response: "OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:EXIT_GAME\r\nLobby ID:lobby_id\r\n\r\n"
      } catch (...) {
        cerr << "Invalid number of parameters for action: " + inputs.at(0) << endl;
      }
      break;
    case 4:
      // E.g. "LIST\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n"
      res = inputs.at(0) + "\r\nAck:" + to_string(ack) + "\r\nPlayer ID:" + player_id + "\r\n\r\n";
      // E.g. Response: "OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:LIST\r\nNumber of Lobby:x\r\nLobby ID:lobby_id\r\nLobby Name:lobby_name\r\n\r\n"
      break;
    case 5:
    case 6:
      // E.g. "JOIN lobby_id\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n"
      res = inputs.at(0) + " " + inputs.at(1) + "\r\nAck:" + to_string(ack) + "\r\nPlayer ID:" + player_id + "\r\n\r\n";
      // E.g. Response: "OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:JOIN\r\nLobby ID:lobby_id\r\nLobby Name:lobby_name\r\n\r\n"
  }
  return res;
}

ActionResponse performAction(const addrinfo* servInfo, const string inputAction, int& ack, const string player_id, int isDebug) {
  ActionResponse res;
  // Format an action
  string request = formatRequest(inputAction, ack, player_id);

  if (request == "") {
    cout << "Invalid action" << endl;
    res.status = 0;
    return res;
  }
  try {
    // Create client socket
    printDebugStatement(isDebug, "Connecting to server with Player ID: " + player_id);
    int clientSd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);

    // Connect to server and check connection status
    int status = connect(clientSd, servInfo->ai_addr, servInfo->ai_addrlen);
    if (status < 0) {
      cerr << "Failed to connect to the server" << endl;
      close(clientSd);
      res.status = 0;
      return res;
    }

    // Write the action to server
    printDebugStatement(isDebug, "Request:\n" + request);
    int writeRes = write(clientSd, request.c_str(), strlen(request.c_str()));
    if (writeRes <= 0) {
      cerr << "Unable to send the request" << endl;
      res.status = 0;
      return res;
    }

    // Read response from server
    // Get Server response
    printDebugStatement(isDebug, "Request Response: ");
    try {
      while (true) {
        string responseHeader = parseResponseInfo(clientSd);
        if (responseHeader == "") {
          break;
        }
        printDebugStatement(isDebug, "Response Header:\n" + responseHeader);
        if (responseHeader.substr(0,2) == "OK") {
          res.status = 1;
        }
        else if (responseHeader.substr(0,2) == "NO") {
          res.status = 0;
        }
        else if (responseHeader.substr(0, 5) == "Ack:") {
          int ack_recv = atoi(responseHeader.substr(5, responseHeader.length()).c_str());
          if (ack_recv == ack) {
            ack++;
          }
        }
        else if (responseHeader.substr(0, 10) == "Player ID:") {
          string player_id_recv = responseHeader.substr(10, responseHeader.length()).c_str();
          res.player_id = player_id_recv;
        }
        else if (responseHeader.substr(0, 7) == "Action:") {
          res.action = responseHeader.substr(7, responseHeader.length()).c_str();
        }
        else {
          res.information.push_back(responseHeader);
        }
      }
    } catch (...) {
      cerr << "Error happened in reading server response to perform request." << endl;
      close(clientSd);
      res.status = 0;
      return res;
    }
    close(clientSd);
  } catch (...) {
      res.status = 0;
      return res;
  }
  return res;
}


struct GetUsernameReturn {
  string player_id;
  string username;
};

GetUsernameReturn getValidUsername(const addrinfo* servInfo, int& ack, int isDebug) {
  GetUsernameReturn res;
  bool isUsernameSet = false;
  try {
    while (!isUsernameSet) {
      // Ask player for username
      string username = "";
      cout << "Please type your username (no space, no symbol), press ENTER to submit: ";
      getline(cin, username);

      // Create client socket
      cout << "Try Joining server with username: " + username << endl;
      int clientSd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);

      // Connect to server and check connection status
      int status = connect(clientSd, servInfo->ai_addr, servInfo->ai_addrlen);
      if (status < 0) {
        cerr << "Failed to connect to the server" << endl;
        close(clientSd);
        continue;
      }

      // Make a create username request to server
      string request = string("CREATE_USER " + string(username) + "\r\n" + "Ack:" + to_string(ack) + "\r\n\r\n");
      // E.g. Response "OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:CREATE_USER\r\n"
      printDebugStatement(isDebug, "Request:\n" + request);

      int writeRes = write(clientSd, request.c_str(), strlen(request.c_str()));
      if (writeRes <= 0) {
        cerr << "Unable to send the request" << endl;
        close(clientSd);
        continue;
      }

      // TODO: Set timer

      // Get Server response
      printDebugStatement(isDebug, "Request Response: ");
      try {
        while (true) {
          string responseHeader = parseResponseInfo(clientSd);
          if (responseHeader == "") {
            break;
          }
          printDebugStatement(isDebug, "Response Header:\n" + responseHeader);
          if (responseHeader.substr(0,2) == "NO") {
            cout << "Invalid Username. Please retry." << endl;
          }
          if (responseHeader.substr(0,2) == "OK") {
            res.username = username;
            isUsernameSet = true;
          }
          if (responseHeader.substr(0, 5) == "Ack:") {
            int ack_recv = atoi(responseHeader.substr(5, responseHeader.length()).c_str());
            if (ack_recv == ack) {
              ack++;
            }
          }
          if (responseHeader.substr(0, 10) == "Player ID:") {
            string player_id_recv = responseHeader.substr(10, responseHeader.length()).c_str();
            res.player_id = player_id_recv;
          }
        }
      } catch (...) {
        cerr << "Error happened in reading server response to create username." << endl;
        close(clientSd);
        continue;
      }
      close(clientSd);
    }
  } catch (...) {
    cerr << "Error happened in create username." << endl;
  }
  printDebugStatement(isDebug, "Username: " + res.username + ", Player ID: " + res.player_id);
  return res;
}

int main(int argc, char* argv[]) {
  // Validate parameters
  if (argc != 4) { return -1; }
  char* serverName;
  char* serverPort;
  int isDebug;
  try {
    serverName = argv[1];
    serverPort = argv[2];
    isDebug = stoi(argv[3]);
  } catch (...) {
    cerr << "Error happened getting parameter" << endl;
    return -1;
  }

  // Load address structs with getaddrinfo
  struct addrinfo hints, *servInfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Check server address status
  int serverAddrInfoStatus = getaddrinfo(serverName, serverPort, &hints, &servInfo);
  if (serverAddrInfoStatus != 0) {
    cerr << "Unable to connect" << endl;
    return -1;
  }

  int ack = 0; // ACK
  bool isJoined = false;

  // Setup Player account
  GetUsernameReturn res = getValidUsername(servInfo, ack, isDebug);
  string player_id = res.player_id;
  string username = res.username;

  // Start the game
  if (!player_id.empty() && !username.empty()) {
    isJoined = true;
  }
  // Start by loading a list of lobby
  try {

    while (isJoined) {
      // Read Player input
      string input;
      cout << "Type action (Type 'HELP' for list of command): ";
      getline(cin, input);

      // Perform action based on input
      ActionResponse res = performAction(servInfo, input, ack, player_id, isDebug);
      if (res.status == 0 || input.substr(0,4) == "HELP") {
        // Print a list of command
        continue;
      }

      // Process response
      if (res.action == "EXIT") {
        return 0;
      }
    }
  } catch (...) {

  }
}
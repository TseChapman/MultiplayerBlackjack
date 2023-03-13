# MultiplayerBlackjack

## Project Documentation
To see how to play the game please go to “Run Instruction”, command and response below is a simplified HTTP protocol that player would not run.

### Client
Player uses the client as a sender to send command to the server and wait for server to response the result of the command. When the client starts, it ask the player to type a unused username. Once a valid username is assigned, player can perform by typing valid commands. 
#### Write Client Request
There are a few commands that need to write to the server:
| Command     |	Description                                                                                                          | Example Format                                                              |
|-------------|----------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------|
| EXIT        | this command asks the server to exit any lobby and unregister the username                                           | EXIT\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n                              |
| CREATE_GAME | this command asks the server to create a lobby with the inputted lobby name                                          | CREATE_GAME lobby_name\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n            |
| EXIT_GAME   | this command asks the server to exit any lobby that the player has joined                                            | EXIT_GAME lobby_id\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n                |
| LIST        | this command asks the server to return a list of lobbies that are ready for players to join                          |	LIST\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n                             |
| CREATE_USER | this command asks the server to register an unused username, invalid after a username is assigned to the client      |	CREATE_USER username\r\nAck:ack\r\n\r\n                                    |
| JOIN        | this command asks the server to let the player join a lobby by its lobby id                                          |	JOIN lobby_id\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n                    |
| hit         | this command can only be used when the player has joined a lobby, draw a card if it is the player’s turn	           | GAME hit\r\nAck:ack\r\nPlayer ID:player_id\r\nLobby ID:lobby_id\r\n\r\n     |
| stand       | this command can only be used when the player has joined a lobby, don’t draw anymore card if it is the player’s turn | GAME stand\r\nAck:ack\r\nPlayer ID:player_id\r\nLobby ID:lobby_id\r\n\r\n   |
| updates     | this command can only be used when the player has joined a lobby, get the status of the lobby                      	 | GAME updates\r\nAck:ack\r\nPlayer ID:player_id\r\nLobby ID:lobby_id\r\n\r\n |
| SCORE       | this command asks the server to return the current global scoreboard of the game             	                       | SCORE\r\nAck:ack\r\nPlayer ID:player_id\r\n\r\n                             |

#### Process Server Response
When server write back a response, the status of the response can be checked on the first header line. If the first 2 characters is OK, then the request is processed successfully. If NO, then the player send an invalid request to server.
Based on the response information, the client will print the differently to the terminal:
| Command                 |	Example Print                                                                                                                             |
|-------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
| EXIT	                  | Action: EXIT<br>Exiting Game                                                                                                              |
| CREATE_GAME             |	Action: CREATE_GAME<br>Create lobby with lobby id: 0                                                                                      |
| EXIT_GAME               |	Action: EXIT_GAME<br>Exit lobby with lobby id: 0                                                                                          |
| LIST                    |	Action: LIST<br>Number of lobbies: 1<br>Lobby ID&emsp;Lobby Name<br>0&emsp;&emsp;&emsp;&emsp;&nbsp;hello                                  |
| CREATE_USER             |	Try Joining server with username: demo<br>Successfully register user with username: demo                                                  |
|JOIN                   	| Action: JOIN<br>Joined Lobby with Lobby id: 0                                                                                             |
| hit<br>stand<br>updates |	Game Action: hit/stand/updates<br>Current turn: PLAYER_1_TURN<br>Dealer's Hand:2_Diamonds = Score: 2<br>Player 0 Hands:4_Clubs = Score: 4 |
| SCORE                   |	Player: username, Wins = 0                                                                                                                |

### Server
The server act as a processing center for client request on the game. The server will create a new thread whenever a client connects and end when one’s request is processed and returned a response.

#### Process Client Request
The server will act differently based on the command. All of the request requires mutex lock to ensure no race condition happened.
| Command	                | Description                                                                                                                                                                                           |
|-------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| EXIT	                  | Based on the player id, check if the player is currently in a lobby, if so, exit the lobby. Then, unregister the username                                                                             |
| CREATE_GAME	            | Check if the game is created, if not create a new Game object and assign a lobby id to the game                                                                                                       |
| EXIT_GAME             	| Based on the inputted lobby id, check if the player is joined, if so, exit the lobby                                                                                                                  |
| LIST	                  | Get a list of lobby, check if the lobby is ready to join. Count the number of lobbies that are ready for join and return the corresponding id and name to the client                                  |
| CREATE_USER	            | Get the list of players and check if the username is taken. If not, add the new Player to the list and insert username and assign new player id to the Player                                         |
| JOIN	                  | Get a list of lobbies and check if there is a lobby id that matches the inputted lobby id. If so, check if the lobby is ready for join and it is not full. If not full, add the player id to the Game |
| hit<br>stand<br>updates |	Find the corresponding lobby id from a list of Games and pass the game action to the Game                                                                                                             |
| SCORE                   |	Get a list of players and for each player, check the game history and record wins for each game ended                                                                                                 |

#### Write Server Response
There are 2 categories of response based on if the request is processed successfully or not.<br>If the request is processed and performed successfully, the general response is “OK\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:action\r\ninformation\r\n\r\n”, action is the command inputted by the client, the information at the end of the response is any additional information required by the request. For example, CREATE_GAME would need to return the lobby id correspond to the new game created.
<br>If the request is processed unsuccessfully, the general response is “NO\r\nAck:ack\r\nPlayer ID:player_id\r\nAction:action\r\ninformation\r\n\r\n”, action is the command inputted by the client, the information at the end of the response is any additional information required by the request. For example, use CREATE_USER on an used username, would return invalid command response back to the client
#### Game
The Game class is an object that is used to perform game related command (hit, stand, updates) and other lobby related activities (join, exit lobby, etc.).
| Functions         |	Description |
|-------------------|-------------|
| action            |	Based on the game action (hit, stand, updates) perform the following<br>-	Hit: draw a card from the deck if it is the players turn, then move to the next stage<br>-	Stand: mark the player as stand and move to the next stage<br>-	Updates: nothing happens, return the current status of the game<br>The game has multiple stages, player, dealer, and determining stage. Player stage is to wait until the corresponding player make a move. Dealer stage is to draw a card until the hand score is greater than 17. If greater than 17, stand the dealer hand. If exceed, move to determining stage for determining winner. Determining stage is the determine if the game continue or the winner(s) is determined |
| join              |	Check if the player already joined, if not, check if the lobby is full, if not, allow the player to join |
| exit              |	Check if the player joined the lobby, if so, erase the player information (hand, players list) in the lobby |
| isFull	          | Return if the lobby is full |
| getCurrentStatus	| Get the string representation of the current stage (Player, Dealer, Determine)	| 
| isPlayerWin	      | Pass in the player id and check if the player wins	| 
| getPlayerIndex	  | Get the player index (not id) in the players list	| 
| isPlayerJoined	  | Check if the player joined the lobby in the players list	| 
| printHand        	| Get a string representation of the hand the player have	| 
| countScore      	| Count the score of a hand	| 
#### Deck
This class is to store the deck in each game. The cards are represented by its number and its suit. These are cards are shuffled and ready to be drawn from the game.
| Function	| Description                                  |
|-----------|----------------------------------------------|
| drawCard  |	Draw a card from the bottom of the deck      |
| shuffle	  | Use Fisher-Yates shuffle to shuffle the deck |
### Run Instruction
1.	Use ./build.sh to build the game.
2.	Use ./runServer.sh port# to run the server.
3.	Use ./runClient.sh server_name port# to run the client.
4.	The Client will first ask for the valid username (no space, no symbol)
5.	Once registered a username, the client can perform the following commands:
a.	HELP, get a list of command with example
b.	EXIT, will the exit the game
c.	CREATE_GAME lobby_name, create a lobby with lobby name “lobby_name”. Game requires 4 players to start
d.	EXIT_GAME, exit the lobby (if joined one)
e.	LIST, to get t a list of lobbies that are ready for player to join
f.	JOIN lobby_id, join a lobby with specific valid lobby id, lobby id can be found by using the LIST
g.	SCORE, view the global scoreboard
h.	Game action: hit, stand, updates.
i.	hit: draw a card, if it is the player’s turn
ii.	stand: stop drawing anymore card, if it is the player’s turn
iii.	updates: get the current status of the game, *recommended to use this before making any move (hit or stand) in the game 
6.	Once done with the game use EXIT to exit client
7.	Ctrl C to execute server

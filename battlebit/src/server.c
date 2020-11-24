//
// Created by carson on 5/20/20.
//

#include "stdio.h"
#include "stdlib.h"
#include "server.h"
#include "char_buff.h"
#include "game.h"
#include "repl.h"
#include "pthread.h"
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h>    //inet_addr
#include<unistd.h>    //write

static game_server *SERVER;

void init_server() {
    if (SERVER == NULL) {
        SERVER = calloc(1, sizeof(struct game_server));
    } else {
        printf("Server already started");
    }
}

int handle_client_connect(int player) {
    // STEP 9 - This is the big one: you will need to re-implement the REPL code from
    // the repl.c file, but with a twist: you need to make sure that a player only
    // fires when the game is initialized and it is there turn.  They can broadcast
    // a message whenever, but they can't just shoot unless it is their turn.
    //
    // The commands will obviously not take a player argument, as with the system level
    // REPL, but they will be similar: load, fire, etc.
    //
    // You must broadcast informative messages after each shot (HIT! or MISS!) and let
    // the player print out their current board state at any time.
    //
    // This function will end up looking a lot like repl_execute_command, except you will
    // be working against network sockets rather than standard out, and you will need
    // to coordinate turns via the game::status field.
    int opponent = (player + 1) % 2;
    game *game = game_get_current();
    char msg[100];
    char raw_buffer[2000];
    char_buff *input_buffer = cb_create(2000);
    char_buff *output_buffer = cb_create(2000);

    int read_size;
    sprintf(output_buffer->buffer, "Welcome to battleBit server Player %d\nbattleBit (? for help) > ", player);
    cb_write(SERVER->player_sockets[player], output_buffer);

    while ((read_size = recv(SERVER->player_sockets[player], raw_buffer, 2000, 0)) > 0) {
        cb_reset(output_buffer);
        cb_reset(input_buffer);
        if (read_size > 0) {
            raw_buffer[read_size] = '\0';
            cb_append(input_buffer, raw_buffer);
            char *command = cb_tokenize(input_buffer, " \r\n");
            if (strcmp(command, "?") == 0) {
                cb_append(output_buffer, "? - show help\n");
                cb_append(output_buffer, "load <string> - load a ship layout file for the given player\n");
                cb_append(output_buffer, "show - shows the board for the given player\n");
                cb_append(output_buffer, "fire [0-7] [0-7] - fire at the given position\n");
                cb_append(output_buffer, "say <string> - Send the string to all players as part of a chat\n");
                cb_append(output_buffer, "quit - quit the server\n");
                cb_write(SERVER->player_sockets[player], output_buffer);
            } else if (strcmp(command, "load") == 0) {
                if(game_load_board(game, player, cb_next_token(input_buffer)) == -1){
                    cb_append(output_buffer, "Invalid Spec\n");
                    server_broadcast(output_buffer, player);
                } else if(SERVER->player_threads[1]){
                    game->status = PLAYER_0_TURN;
                    cb_append(output_buffer, "All Player Boards Loaded\nPlayer 0 Turn\n");
                    server_broadcast(output_buffer, player);
                } else {
                    game->status = INITIALIZED;
                    cb_append(output_buffer, "Waiting on Player 1\n");
                    server_broadcast(output_buffer, player);
                }
            } else if (strcmp(command, "show") == 0) {
                repl_print_hits(&game->players[player], output_buffer);
                cb_write(SERVER->player_sockets[player], output_buffer);
                repl_print_ships(&game->players[player], output_buffer);
                cb_write(SERVER->player_sockets[player], output_buffer);
            } else if (strcmp(command, "fire") == 0) {
                if(game->status == CREATED){
                    cb_append(output_buffer, "Game Has Not Begun!\n");
                    cb_write(SERVER->player_sockets[player], output_buffer);
                } else if(game->status == PLAYER_0_TURN && player == 1){
                    cb_append(output_buffer, "Player 0 Turn\n");
                    cb_write(SERVER->player_sockets[player], output_buffer);
                } else if(game->status == PLAYER_1_TURN && player == 0){
                    cb_append(output_buffer,"Player 1 Turn\n");
                    cb_write(SERVER->player_sockets[player], output_buffer);
                } else {
                    int x = cb_next_token(input_buffer)[0] - '0';
                    int y = cb_next_token(input_buffer)[0] - '0';
                    if (game_fire(game, player, x, y) == 0) {
                        sprintf(msg, "Player %d fires at %d %d - Miss\n", player, x, y);
                        cb_append(output_buffer, msg);
                        server_broadcast(output_buffer, player);
                    } else {
                        if(game->players[opponent].ships == 0ull){
                            sprintf(msg, "Player %d fires at %d %d - Hit - Player %d Wins!\n", player, x, y, player);
                            cb_append(output_buffer, msg);
                            server_broadcast(output_buffer, player);
                            close(SERVER->player_sockets[player]);
                            close(SERVER->player_sockets[opponent]);
                        } else {
                            sprintf(msg, "Player %d fires at %d %d - Hit\n", player, x, y);
                        }
                        cb_append(output_buffer, msg);
                        server_broadcast(output_buffer, player);
                    }
                }
            } else if (strcmp(command, "say") == 0) {
                sprintf(msg, "\nPlayer %d says:%s", player, raw_buffer + 3);
                cb_append(output_buffer, msg);
                cb_append(output_buffer, "battleBit (? for help) > ");
                cb_write(SERVER->player_sockets[opponent], output_buffer);
            } else if (strcmp(command, "quit") == 0) {
                close(SERVER->player_sockets[player]);
            } else {
                cb_append(output_buffer, "Invalid Command\n");
                cb_write(SERVER->player_sockets[player], output_buffer);
            }
            cb_reset(output_buffer);
            cb_append(output_buffer, "battleBit (? for help) > ");
            cb_write(SERVER->player_sockets[player], output_buffer);
        }
    }
    return 0;
}

void server_broadcast(char_buff *msg, int player) {
    // send message to all players
    int opponent = (player + 1) % 2;
    char_buff *temp = cb_create(2000);
    cb_write(SERVER->player_sockets[player], msg);
    sprintf(temp->buffer, "\n%sbattleBit (? for help) > ", msg->buffer);
    cb_write(SERVER->player_sockets[opponent], temp);
    printf("%s", temp->buffer);
}

int run_server() {
    // STEP 8 - implement the server code to put this on the network.
    // Here you will need to initalize a server socket and wait for incoming connections.
    //
    // When a connection occurs, store the corresponding new client socket in the SERVER.player_sockets array
    // as the corresponding player position.
    //
    // You will then create a thread running handle_client_connect, passing the player number out
    // so they can interact with the server asynchronously
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_fd == -1) {
        printf("Could not create socket\n");
    }

    int yes = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in server;

    // fill out the socket information
    server.sin_family = AF_INET;
    // bind the socket on all available interfaces
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9876);

    if (bind(server_socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        puts("Bind failed");
    } else {
        puts("Bind worked!");
        listen(server_socket_fd, 88);

        //Accept an incoming connection
        puts("Waiting for incoming connections...");

        struct sockaddr_in client;
        socklen_t size_from_connect;
        int client_socket_fd;
        int request_count = 0;
        game_get_current()->status = CREATED;
        while ((client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client, &size_from_connect)) > 0) {
            SERVER->player_sockets[request_count] = client_socket_fd;
            pthread_create(&SERVER->player_threads[request_count], NULL, (void *) handle_client_connect, (void *) request_count);
            request_count++;
        }
    }
    return 0;
}

int server_start() {
    // STEP 7 - using a pthread, run the run_server() function asynchronously, so you can still
    // interact with the game via the command line REPL
    init_server();
    pthread_create(&SERVER->server_thread, NULL, (void *) run_server, NULL);
    return 0;
}

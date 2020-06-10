#include "api.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "srb2"
#define MAX_CLIENTS 1
#define REQUEST_SIZE 1024
#define NO_SOCKET 0

int api_socket;
client_t connection_list[MAX_CLIENTS];

void API_init(void) {
    printf("API_init\n");

    // Initialize client connection list to "no clients connected"
    for (int i = 0; i < MAX_CLIENTS; i++) {
        connection_list[i].socket = NO_SOCKET;
    }

    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    char *socket_path = malloc(strlen(runtime_dir) + strlen(SOCKET_NAME) + 2);
    sprintf(socket_path, "%s/%s", runtime_dir, SOCKET_NAME);

    api_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (api_socket == -1) {
        perror("Opening socket");
    }
    if (unlink(socket_path)) {
        perror("Unlinking path");
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    if (bind(api_socket, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("Binding socket");
    }

    if (listen(api_socket, 0)) {
        perror("Listening");
    }
    printf("Started listening on socket: %s\n", socket_path);
    while (1) {
        api_check_socket();
    }
    exit(0);
}

void api_handle_connection() {
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&client_addr, 0, client_len);
    int client_sock =
        accept(api_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept()");
        return;
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connection_list[i].socket == NO_SOCKET) {
            connection_list[i].socket = client_sock;
            return;
        }
    }
    dprintf(STDERR_FILENO, "Too many clients connected\n");
}

void api_send_response(client_t *api_client, char *response) {
    int written = write(api_client->socket, response, strlen(response)); 
    if (written == -1) {
        perror("Writing response");
        return;
    }
}

void api_handle_request(client_t *api_client, char *request) {
    printf("Request: %s", request);
    api_send_response(api_client, request);
}

void api_receive(client_t *api_client) {
    char request[REQUEST_SIZE];
    int num_read = read(api_client->socket, request, REQUEST_SIZE-1);
    if (num_read == -1) {
        perror("reading request");
        return;
    }
    if (num_read == 0) {
        //End of file
        api_close_connection(api_client);
        return;
    }
    //Terminate the string
    request[num_read] = '\0';
    api_handle_request(api_client, request);
}

void api_close_connection(client_t *api_client) {
    if (close(api_client->socket) == -1) {
        perror("closing socket");
        return;
    }
    api_client->socket = NO_SOCKET;
}

void api_check_socket() {
    int maxfd = api_socket;
    fd_set rfds, efds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(api_socket, &rfds);

    FD_ZERO(&efds);
    FD_SET(api_socket, &efds);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_t *api_client = &connection_list[i];
        if (api_client->socket != NO_SOCKET) {
            if (api_client->socket > maxfd) {
                maxfd = api_client->socket;
            }
            FD_SET(api_client->socket, &rfds);
            FD_SET(api_client->socket, &efds);
        }
    }

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    retval = select(maxfd + 1, &rfds, NULL, &efds, &tv);

    if (retval == -1) {
        perror("select()");
    } else if (retval) {
        if (FD_ISSET(api_socket, &rfds)) {
            api_handle_connection();
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            client_t *api_client = &connection_list[i];
            if (api_client->socket != NO_SOCKET) {
                if (FD_ISSET(api_client->socket, &rfds)) {
                    api_receive(api_client);
                } else if (FD_ISSET(api_client->socket, &efds)) {
                    perror("Client exception");
                    api_close_connection(api_client);
                }
            }
        }
    }
}

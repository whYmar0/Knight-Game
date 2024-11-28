#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SQUARE_SIZE 50

typedef struct {
    int x, y; // Координаты квадрата
} Square;

void move_square(Square *square, char command) {
    switch (command) {
        case 'w': square->y -= 10; break;
        case 's': square->y += 10; break;
        case 'a': square->x -= 10; break;
        case 'd': square->x += 10; break;
    }
    if (square->x < 0) square->x = 0;
    if (square->y < 0) square->y = 0;
    if (square->x > SCREEN_WIDTH - SQUARE_SIZE) square->x = SCREEN_WIDTH - SQUARE_SIZE;
    if (square->y > SCREEN_HEIGHT - SQUARE_SIZE) square->y = SCREEN_HEIGHT - SQUARE_SIZE;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket binding failed");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server is running and waiting for clients...\n");

    client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_socket < 0) {
        perror("Client connection failed");
        return 1;
    }

    printf("Client connected. Waiting for commands...\n");

    Square square = {SCREEN_WIDTH / 2 - SQUARE_SIZE / 2, SCREEN_HEIGHT / 2 - SQUARE_SIZE / 2};

    char command;
    while (1) {
        ssize_t received = recv(client_socket, &command, sizeof(command), 0);
        if (received > 0) {
            if (command == 'q') break;
            move_square(&square, command);
            send(client_socket, &square, sizeof(square), 0); // Отправляем обновленные координаты
        }
    }

    close(client_socket);
    close(server_fd);
    return 0;
}

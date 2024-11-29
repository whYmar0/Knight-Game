#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Функция для обработки каждого клиента
void *client_handler(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];

    // Чтение данных от клиента
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            printf("Клиент отключился\n");
            break;
        }
        printf("Получено от клиента: %s\n", buffer);

        // Отправка ответа клиенту
        char *response = "Сообщение получено!";
        send(client_socket, response, strlen(response), 0);
    }

    // Закрытие соединения
    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Создание сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязка сокета к порту
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка привязки");
        exit(1);
    }

    // Ожидание подключения клиентов
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Ошибка при ожидании подключения");
        exit(1);
    }

    printf("Сервер запущен и ожидает подключения клиентов...\n");

    // Основной цикл сервера
    while (1) {
        // Прием подключения клиента
        if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len)) < 0) {
            perror("Ошибка при подключении клиента");
            continue;
        }

        printf("Клиент подключен: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Обработка клиента в отдельном потоке
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, &client_socket) != 0) {
            perror("Ошибка при создании потока");
        }

        // Отделение потока
        pthread_detach(client_thread);
    }

    // Закрытие серверного сокета
    close(server_socket);
    return 0;
}

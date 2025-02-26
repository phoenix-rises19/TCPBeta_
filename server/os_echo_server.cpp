#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

int main() {
    // Create a TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket options
    int option = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Bind to port 4444
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9010);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Echo server listening on port"<< server_addr.sin_port;

    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        // Accept a connection
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        std::cout << "Client connected\n";

        // Echo loop
        char buffer[1024];
        ssize_t bytes_read;

        while ((bytes_read = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
            std::cout << "Received " << bytes_read << " bytes\n";
            send(client_fd, buffer, bytes_read, 0);
        }

        std::cout << "Client disconnected\n";
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

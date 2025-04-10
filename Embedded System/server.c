#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

//Lenh build gcc server.c
//Lenh chay sudo ./a.out

#define DALI_DRV "/dev/my_dali_driver"  // Kernel-module C name (in devices dir)
#define PORT 28009
#define BUFFER_SIZE 10

int main(void) {
    int fd;
    unsigned int value;
    char buffer[BUFFER_SIZE];
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Open the DALI driver
    fd = open(DALI_DRV, O_RDWR);
    if (fd < 0) {
        perror("Cannot open device");
        return 1;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation error");
        close(fd);
        return 1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Set socket options error");
        close(fd);
        close(server_fd);
        return 1;
    }

    // Define the address and port for the socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP
    address.sin_port = htons(PORT); // Convert port to network byte order

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(fd);
        close(server_fd);
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(fd);
        close(server_fd);
        return 1;
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            break; // Exit the loop on accept failure
        }

        // Read data from the socket
        int bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the received string
            // Convert string to unsigned integer
            value = strtoul(buffer, NULL, 10); // Convert to unsigned integer
            printf("Received unsigned integer: %u\n", value);

            // Write the received value to the DALI driver as a single byte
            // Ensure that the value fits in a single byte (0-255)
                write(fd, buffer, 1); // Send the value as a single byte
        } else {
            perror("Read failed");
        }

        close(new_socket); // Close the connection with the client
    }

    // Clean up
    close(fd);
    close(server_fd);
    return 0;
}

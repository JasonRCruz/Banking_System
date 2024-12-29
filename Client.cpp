#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include "bank.h"

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>\n";
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = std::stoi(argv[2]);

    // Create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    // Server address setup
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    // Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to the CSE384 Banking System!\n";

    while (true) {
        std::cout << "\nMenu:\n"
                  << "1. Check balance\n"
                  << "2. Deposit money\n"
                  << "3. Withdraw money\n"
                  << "4. Transfer money\n"
                  << "0. Exit\n"
                  << "Enter choice: ";
        std::string choice;
        std::getline(std::cin, choice);

        std::string command;

        if (choice == "1") {
            command = "CHECK";
        } else if (choice == "2") {
            std::cout << "Deposit into which account? (checking/savings): ";
            std::string acct;
            std::getline(std::cin, acct);
            std::cout << "Enter deposit amount: ";
            std::string amt;
            std::getline(std::cin, amt);
            command = "DEPOSIT " + acct + " " + amt;
        } else if (choice == "3") {
            std::cout << "Withdraw from which account? (checking/savings): ";
            std::string acct;
            std::getline(std::cin, acct);
            std::cout << "Enter withdrawal amount: ";
            std::string amt;
            std::getline(std::cin, amt);
            command = "WITHDRAW " + acct + " " + amt;
        } else if (choice == "4") {
            std::cout << "Transfer FROM which account? (checking/savings): ";
            std::string fromAcct;
            std::getline(std::cin, fromAcct);
            std::cout << "Transfer TO which account? (checking/savings): ";
            std::string toAcct;
            std::getline(std::cin, toAcct);
            std::cout << "Enter transfer amount: ";
            std::string amt;
            std::getline(std::cin, amt);
            command = "TRANSFER " + fromAcct + " " + toAcct + " " + amt;
        } else if (choice == "0") {
            command = "EXIT";
        } else {
            std::cout << "Invalid choice.\n";
            continue;
        }

        // Send command to server
        if (send(sock_fd, command.c_str(), command.size(), 0) == -1) {
            perror("send");
            break;
        }

        if (command == "EXIT") {
            // After sending EXIT, break immediately
            std::cout << "Exiting...\n";
            break;
        }

        // Receive response
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_recv = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_recv <= 0) {
            std::cout << "Server disconnected.\n";
            break;
        }
        std::cout << "[Server Response]:\n" << buffer << std::endl;
    }

    close(sock_fd);
    return 0;
}

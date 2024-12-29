#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include "bank.h"

#define BUFFER_SIZE 1024

// Global instance for a single user's bank account (for demonstration).
static BankAccount globalAccount = {0.0, 0.0};

std::string processRequest(const std::string &request, BankAccount &account) {
    // Split input by whitespace
    std::istringstream iss(request);
    std::string command;
    iss >> command;

    if (command == "CHECK") {
        // Check balance
        return "Checking: " + std::to_string(account.checking) +
               "\nSavings:  " + std::to_string(account.savings);
    } 
    else if (command == "DEPOSIT") {
        std::string accountType;
        double amount = 0.0;
        iss >> accountType >> amount;

        if (amount <= 0) {
            return "ERROR: Deposit amount must be > 0";
        }
        if (accountType == "checking") {
            account.checking += amount;
            return "Deposited " + std::to_string(amount) + 
                   " to checking. New balance: " + std::to_string(account.checking);
        } else if (accountType == "savings") {
            account.savings += amount;
            return "Deposited " + std::to_string(amount) +
                   " to savings. New balance: " + std::to_string(account.savings);
        } else {
            return "ERROR: Invalid account type";
        }
    }
    else if (command == "WITHDRAW") {
        std::string accountType;
        double amount = 0.0;
        iss >> accountType >> amount;

        if (amount <= 0) {
            return "ERROR: Withdrawal amount must be > 0";
        }
        if (accountType == "checking") {
            if (account.checking < amount) {
                return "ERROR: Insufficient funds in checking";
            } 
            account.checking -= amount;
            return "Withdrew " + std::to_string(amount) +
                   " from checking. New balance: " + std::to_string(account.checking);
        } else if (accountType == "savings") {
            if (account.savings < amount) {
                return "ERROR: Insufficient funds in savings";
            } 
            account.savings -= amount;
            return "Withdrew " + std::to_string(amount) +
                   " from savings. New balance: " + std::to_string(account.savings);
        } else {
            return "ERROR: Invalid account type";
        }
    }
    else if (command == "TRANSFER") {
        std::string fromAccount, toAccount;
        double amount = 0.0;
        iss >> fromAccount >> toAccount >> amount;

        if (amount <= 0) {
            return "ERROR: Transfer amount must be > 0";
        }
        if (fromAccount == toAccount) {
            return "ERROR: 'from' and 'to' accounts cannot be the same.";
        }

        // Check valid accounts
        if (fromAccount != "checking" && fromAccount != "savings") {
            return "ERROR: Invalid source account.";
        }
        if (toAccount != "checking" && toAccount != "savings") {
            return "ERROR: Invalid destination account.";
        }

        // Check funds
        double *src = (fromAccount == "checking") ? &account.checking : &account.savings;
        double *dest = (toAccount == "checking") ? &account.checking : &account.savings;
        if (*src < amount) {
            return "ERROR: Insufficient funds in " + fromAccount;
        }

        // Perform transfer
        *src -= amount;
        *dest += amount;
        return "Transferred " + std::to_string(amount) + " from " + fromAccount +
               " to " + toAccount + 
               "\nChecking: " + std::to_string(account.checking) +
               "\nSavings:  " + std::to_string(account.savings);
    }
    else if (command == "EXIT") {
        return "EXIT";
    }
    else {
        return "ERROR: Unknown command";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    int port = std::stoi(argv[1]);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // Listen
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << port << "...\n";

    // Accept one client for demonstration (single-threaded approach)
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    std::cout << "Client connected!\n";

    // Communicate
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Client disconnected.\n";
            break;
        }

        std::string clientMsg(buffer);
        // Process the request
        std::string response = processRequest(clientMsg, globalAccount);

        // If command is EXIT, close connection
        if (response == "EXIT") {
            send(client_fd, "Goodbye!\n", 9, 0);
            std::cout << "Client requested exit.\n";
            break;
        }

        // Send response back
        response += "\n"; // newline
        send(client_fd, response.c_str(), response.size(), 0);
    }

    close(client_fd);
    close(server_fd);

    return 0;
}

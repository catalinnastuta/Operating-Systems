// main.cc - A simple shell program
// Author: Catalin Silviu Nastuta
// Date: October 24, 2024
// Purpose: A basic shell that reads, parses, and executes commands

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

// Function to display the shell prompt
void print_prompt() {
    std::cout << "myshell> " << std::flush;
}

// Parse user input into tokens
std::vector<char *> parse_input(std::string input) {
    std::vector<char *> args;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        char *arg = new char[token.length() + 1];
        std::strcpy(arg, token.c_str());
        args.push_back(arg);
    }
    args.push_back(nullptr);  // execvp requires a NULL-terminated array
    return args;
}

// Handle 'cd' command as it doesn't work with execvp
void handle_cd(const std::vector<char *> &args) {
    if (args.size() < 2) {
        std::cerr << "cd: missing argument\n";
    } else if (chdir(args[1]) != 0) {
        perror("cd");
    }
}

// Main function: runs an infinite loop to read, parse, and execute commands
int main() {
    std::string input;

    while (true) {
        print_prompt();
        std::getline(std::cin, input);

        if (input.empty()) continue;  // Ignore empty input

        // Parse the input into arguments
        std::vector<char *> args = parse_input(input);

        if (args[0] == nullptr) continue;  // Skip if no valid command

        if (std::string(args[0]) == "exit") {
            break;  // Exit the shell
        } else if (std::string(args[0]) == "cd") {
            handle_cd(args);
        } else {
            pid_t pid = fork();  // Create a child process
            if (pid < 0) {
                perror("Fork failed");
            } else if (pid == 0) {
                // Child process: execute the command
                if (execvp(args[0], args.data()) == -1) {
                    perror("Command execution failed");
                }
                exit(EXIT_FAILURE);  // Exit if execvp fails
            } else {
                // Parent process: wait for the child to finish
                wait(nullptr);
            }
        }

        // Clean up dynamically allocated memory
        for (auto arg : args) {
            delete[] arg;
        }
    }

    return 0;
}

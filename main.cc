// main.cc - A simple shell program with I/O redirection support
// Author: Catalin Silviu Nastuta
// Date: November 4, 2024
// Purpose: A basic shell that reads, parses, and executes commands with I/O redirection

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

// Function to display the shell prompt
void print_prompt() {
    std::cout << "myshell> " << std::flush;
}

// Parse user input into tokens and check for I/O redirection
std::vector<char *> parse_input(std::string input, std::string &input_file, std::string &output_file) {
    std::vector<char *> args;
    std::istringstream iss(input);
    std::string token;
    
    while (iss >> token) {
        if (token == "<") {
            // Get the next token as the input file
            if (iss >> token) {
                input_file = token;
            } else {
                std::cerr << "Error: Missing input file for redirection\n";
                return args;
            }
        } else if (token == ">") {
            // Get the next token as the output file
            if (iss >> token) {
                output_file = token;
            } else {
                std::cerr << "Error: Missing output file for redirection\n";
                return args;
            }
        } else {
            // Regular argument
            char *arg = new char[token.length() + 1];
            std::strcpy(arg, token.c_str());
            args.push_back(arg);
        }
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

        // Variables for I/O redirection
        std::string input_file, output_file;

        // Parse the input into arguments and check for I/O redirection
        std::vector<char *> args = parse_input(input, input_file, output_file);

        if (args.empty() || args[0] == nullptr) continue;  // Skip if no valid command

        if (std::string(args[0]) == "exit") {
            break;  // Exit the shell
        } else if (std::string(args[0]) == "cd") {
            handle_cd(args);
        } else {
            pid_t pid = fork();  // Create a child process
            if (pid < 0) {
                perror("Fork failed");
            } else if (pid == 0) {
                // Child process: handle I/O redirection
                if (!input_file.empty()) {
                    int fd_in = open(input_file.c_str(), O_RDONLY);
                    if (fd_in < 0) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }

                if (!output_file.empty()) {
                    int fd_out = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_out < 0) {
                        perror("Error opening output file");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                }

                // Execute the command
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

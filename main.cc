// main.cc - A simple shell program with I/O redirection and pipe support
// Author: Catalin Silviu Nastuta
// Date: November 18, 2024
// Purpose: A basic shell that reads, parses, and executes commands with I/O redirection and pipes.

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

// Parse user input into tokens, detecting I/O redirection and pipe operators
std::vector<std::vector<char *>> parse_input(std::string input, std::string &input_file, std::string &output_file) {
    std::vector<std::vector<char *>> commands;
    std::istringstream iss(input);
    std::string token;
    std::vector<char *> args;

    while (iss >> token) {
        if (token == "<") {
            if (iss >> token) {
                input_file = token;
            } else {
                std::cerr << "Error: Missing input file for redirection\n";
                return {};
            }
        } else if (token == ">") {
            if (iss >> token) {
                output_file = token;
            } else {
                std::cerr << "Error: Missing output file for redirection\n";
                return {};
            }
        } else if (token == "|") {
            args.push_back(nullptr); // Terminate current command
            commands.push_back(args);
            args.clear();
        } else {
            char *arg = new char[token.length() + 1];
            std::strcpy(arg, token.c_str());
            args.push_back(arg);
        }
    }
    if (!args.empty()) {
        args.push_back(nullptr); // Terminate the last command
        commands.push_back(args);
    }
    return commands;
}

// Handle 'cd' command as it doesn't work with execvp
void handle_cd(const std::vector<char *> &args) {
    if (args.size() < 2) {
        std::cerr << "cd: missing argument\n";
    } else if (chdir(args[1]) != 0) {
        perror("cd");
    }
}

// Execute a command with optional input/output redirection
void execute_command(const std::vector<char *> &args, const std::string &input_file, const std::string &output_file) {
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

    if (execvp(args[0], const_cast<char *const *>(args.data())) == -1) {
        perror("Command execution failed");
        exit(EXIT_FAILURE);
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

        // Parse the input into commands and arguments
        std::vector<std::vector<char *>> commands = parse_input(input, input_file, output_file);
        if (commands.empty() || commands[0].empty() || commands[0][0] == nullptr) continue;

        if (std::string(commands[0][0]) == "exit") {
            break;  // Exit the shell
        } else if (std::string(commands[0][0]) == "cd") {
            handle_cd(commands[0]);
        } else {
            int num_commands = commands.size();
            int pipes[num_commands - 1][2];  // Create pipes for inter-process communication

            for (int i = 0; i < num_commands; ++i) {
                if (i < num_commands - 1) {
                    if (pipe(pipes[i]) == -1) {
                        perror("Pipe creation failed");
                        exit(EXIT_FAILURE);
                    }
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) {
                    // Child process: Handle piping and redirection
                    if (i > 0) {  // Not the first command
                        dup2(pipes[i - 1][0], STDIN_FILENO);
                        close(pipes[i - 1][0]);
                    }
                    if (i < num_commands - 1) {  // Not the last command
                        dup2(pipes[i][1], STDOUT_FILENO);
                        close(pipes[i][1]);
                    }

                    for (int j = 0; j < num_commands - 1; ++j) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    if (i == 0 && !input_file.empty()) {  // Handle input redirection for the first command
                        int fd_in = open(input_file.c_str(), O_RDONLY);
                        if (fd_in < 0) {
                            perror("Error opening input file");
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);
                    }

                    if (i == num_commands - 1 && !output_file.empty()) {  // Handle output redirection for the last command
                        int fd_out = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd_out < 0) {
                            perror("Error opening output file");
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);
                    }

                    execute_command(commands[i], "", "");  // Execute the current command
                }
            }

            for (int i = 0; i < num_commands - 1; ++i) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            for (int i = 0; i < num_commands; ++i) {
                wait(nullptr);  // Wait for all child processes
            }
        }

        // Clean up dynamically allocated memory
        for (const auto &command : commands) {
            for (auto arg : command) {
                delete[] arg;
            }
        }
    }

    return 0;
}

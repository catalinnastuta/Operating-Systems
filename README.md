# Simple Shell Program

## Author
Catalin Silviu Nastuta

## Date
November 18, 2024

## Purpose
This is a basic shell program written in C++ that accepts user commands, executes them, and displays output. It supports built-in commands like `cd` and external commands through `execvp`.

## How to Compile and Run

1. **Clone the repository**:
git clone <your-repository-url> cd <repository-folder>
2. **Compile the program**:
make
3. **Run the shell**:
./myshell

4. **Clean up binaries**:
make clean


## Supported Commands
- **Built-in commands**:
- `cd <directory>`: Change the current directory
- `exit`: Exit the shell

- **External commands**:
- `/bin/date`: Displays the current date and time
- `mkdir <directory_name>`: Creates a new directory
- `ls -F`: Lists files in the current directory
- `cat <file_name>`: Displays the content of a file
## Pipe Support:
Use to pass the output of one command as the input to another.
Example:
- myshell> cat input.txt | grep keyword | wc -l

## New I/O Redirection Support:
Redirect standard input using <:
./myshell> cat < input.txt

Redirect standard output using >:
./myshell> ls > output.txt

Combine input and output redirection:
./myshell> cat < input.txt > output.txt

## Dependencies
- Linux/Unix-like OS (e.g., macOS, WSL, or VirtualBox with Ubuntu)
- GNU Make
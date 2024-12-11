# Tree Command Implementation in C

## Introduction
This project is an implementation of a limited version of the `tree` command in C. It is part of the System-oriented Programming Portfolio Assignment 05 and demonstrates various skills in C programming, including interaction with the Linux filesystem, dynamic memory management, and multithreading.

## Features
1. **Filesystem Interaction**: Recursively traverses directories to display their structure.
2. **Command-line Options**: Supports flags to customize output (e.g., `-L` to limit depth).
3. **Dynamic Data Structures**: Uses linked lists to manage directory entries.
4. **Concurrency**: Implements multithreading to traverse multiple directories simultaneously.
5. **Pipeline Support**: Integrates with Linux pipelines via stdin and stdout.
6. **Thread Safety**: Ensures safe access to shared resources using mutexes.

## Requirements
- GCC compiler
- Linux environment

## Build and Run Instructions
### Prerequisites
Ensure you have the following installed:
- GCC
- Make

### Clone the Repository
```bash
git clone <repository_url>
cd <repository_directory>
```

### Build the Project
Use the provided Makefile to build the project:
```bash
make all
```

### Run the Program
Run the program with appropriate command-line arguments:
```bash
./tree [options] [directory_path]
```
Example:
```bash
./tree -L 2 /home/user
```

### Clean the Build
To clean the build artifacts, use:
```bash
make clean
```

## Command-line Options
- `-L <depth>`: Limit the display to the specified depth.
- `-a`: Include hidden files in the output.
- `-h`: Display usage information.

## Multithreading
The program uses threads to handle directory traversal in parallel. Shared data structures are protected with mutexes to prevent race conditions.

## Pipeline Integration
The program can integrate with other Linux commands. For example:
```bash
./tree | grep "pattern"
```

## Testing
Test cases are included in the `tests/` directory. Run all tests using:
```bash
make test
```

## Contributors
- [Jasmin Wander] https://github.com/xjasx4
- [Finn Krappitz] https://github.com/hendrickson187
- [Tom Knoblach] https://github.com/Gottschalk125

---

For any questions, please contact the contributors via the Git repository.


# Tree Command Implementation in C

## Introduction
This project is an implementation of a limited version of the `tree` command in C. It is part of the System-oriented Programming Portfolio Assignment 05 and demonstrates various skills in C programming, including interaction with the Linux filesystem, dynamic memory management, and multithreading.

## Features
1. **Filesystem Interaction**: Recursively traverses directories to display their structure.
2. **Command-line Options**: Supports flags to customize output (e.g., `-L` to limit depth).
3. **Dynamic Data Structures**: Uses linked lists and trees to manage directory entries.
4. **Concurrency**: Implements multithreading to traverse multiple directories simultaneously and build tree structure for json and csv format.
5. **Pipeline Support**: Integrates with Linux pipelines via stdin and stdout.
6. **Thread Safety**: Ensures safe access to shared resources using mutexes.

## Requirements
- GCC compiler
- Linux or a MacOs environment

## Build and Run Instructions
### Prerequisites
Ensure you have the following installed:
- GCC
- Make

You can check these requierments with the following commands in your terminal:
```bash
gcc --version

make --version
```
In case you haven't installed these programms you can download and install them via these two terminal commands:

On Mac use:
```bash
brew install gcc make
```
On a Linux systems please use:
```bash
sudo apt update
```
```bash
sudo apt install build-essential
```

Now you can check again with the commands from the beginning thate everything went right.




### Clone the Repository
```bash
git clone https://github.com/jshProgrammer/TreeTerminalCommand.git
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
Examples:

If you want to get an overview about the funtions simple use:
```bash
./tree -h
```
For limiting the depth of the tree: 
```bash
./tree -L 2
```
If you want to save the terminal result in a .txt file: 
```bash
./tree -o output.txt
```
To prune for example a specific directory use: 
```bash
./tree --prune ./cmake-build-debug
```

### Clean the Build
To clean the build artifacts, use:
```bash
make clean
```

## Command-line Options
- `-f`: Show full path.
- `-L <depth>`: Limit the depth of the tree.
- `-l`: Follow symbolic links.
- `-a`: Include hidden files in the output.
- `-s`: Show file sizes.
- `--noSum`: Show no summary.
- `-d`: List directories only.
- `--dirsfirst`: List directories before files.
- `-r`: Sort output in reverse order.
- `-t`: Sort by last modification time.
- `-u`: Show username or UID if no name is available.
- `-g`: Show group name or GID if no name is available.
- `-p <dir>`: Prune (omit) specified directory from the tree.
- `--prune <dir>`: Prune (omit) specified directory from the tree.
- `--filelimit #`: Limit descending into directories with more than # entries.
- `--output-json <file>`: Output result in JSON format and send to file.
- `--output-csv <file>`: Output result in CSV format and send to file.
- `-o <file>`: Output result in '.txt'-file.
- `-h`: Show this help message.


## Multithreading
The program uses threads to handle directory traversal in parallel. Shared data structures including queues and trees for CSV-/JSON-filestream are protected with mutexes to prevent race conditions.

## Pipeline Integration
The program can integrate with other Linux commands. For example:
```bash
./tree | grep "tree"
```

## Testing
Test cases are included in the `tests.c` file . Run all tests using:
```bash
make test
```

## Contributors
- [Jasmin Wander] https://github.com/xjasx4
- [Finn Krappitz] https://github.com/hendrickson187
- [Tom Knoblach] https://github.com/Gottschalk125
- [Joshua Pfennig] https://github.com/jshProgrammer

---

For any questions, please contact the contributors via the Git repository.


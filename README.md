# Shell Implementation

A simple Unix-like shell implementation in C with support for command execution, pipelines, file redirection, and built-in commands.

## Features

- **Command Execution**: Execute standard Unix commands
- **Pipelines**: Support for piping commands using the `|` operator
- **Input/Output Redirection**: Redirect input and output using `<` and `>` operators
- **Built-in Commands**:
  - `cd`: Change directory
  - `exit`: Exit the shell
  - `history`: Display command history
  - `tree`: Display file system tree structure

## Project Structure

- `main.c`: Entry point for the shell program
- `shell.h`: Header file containing data structures and function declarations
- `shell.c`: Implementation of shell functionality
- `colors.h`: Color definitions for terminal output

## Data Structures

### Command Structure

The `Command` structure represents a single command in the shell:

```c
typedef struct Command {
  int argc;                // Number of arguments
  char *name;              // Command name
  char *argv[MAX_ARGS];    // Command arguments
  bool is_out_redirect;    // Flag for output redirection
  bool is_in_redirect;     // Flag for input redirection
  char *in_file_name;      // Input file name
  char *out_file_name;     // Output file name
  struct Command *next;    // Pointer to next command in pipeline
} Command;
```

### History Structure

The `History` structure maintains command history:

```c
typedef struct History {
  char *history[HISTORY_LEN];  // Array of history entries
  int start;                   // Start index
  int count;                   // Count of entries
  int index;                   // Current index
} History;
```

## How It Works

### Command Parsing

1. The shell reads input from the user
2. Input is parsed to identify commands, pipes, and redirections
3. A linked list of `Command` structures is created

### Command Execution

1. For each command in the pipeline:
   - Check if it's a built-in command
   - Set up pipes if necessary
   - Set up redirections if necessary
   - Fork a new process
   - Execute the command in the child process
   - Wait for the command to finish in the parent process

### Built-in Commands

- `cd`: Changes the current working directory
- `exit`: Exits the shell
- `history`: Displays command history from history.txt
- `tree`: Displays a tree visualization of the current directory structure

### File Redirection

- Input redirection (`<`): Redirects input from a file
- Output redirection (`>`): Redirects output to a file

### Piping

Pipes are implemented using the `pipe()` system call and connecting the output of one command to the input of the next command.

## Compilation and Execution

Compile the shell with:

```bash
gcc -o myshell main.c shell.c -std=c99
```

Run the shell:

```bash
./myshell
```

## Command Examples

1. Basic command:
   ```
   ls -la
   ```

2. Pipeline:
   ```
   ls -la | grep ".txt" | wc -l
   ```

3. Redirection:
   ```
   ls -la > output.txt
   cat < input.txt
   ```

4. Change directory:
   ```
   cd /path/to/directory
   ```

5. View directory tree:
   ```
   tree
   ```

6. View command history:
   ```
   history
   ```

7. Exit the shell:
   ```
   exit
   ```

## Limitations

- Limited handling of special characters and quotes
- No support for environment variable expansion
- No job control
- No command aliasing
- Limited error handling

## Future Improvements

- Add support for job control
- Implement command aliasing
- Add tab completion
- Add support for environment variable expansion
- Improve error handling
- Add signal handling

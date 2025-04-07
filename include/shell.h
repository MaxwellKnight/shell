#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>

/***********************************************
 * CONSTANTS AND DEFINITIONS
 ***********************************************/
#define MAX_ARGS 20
#define INPUT_LEN 256
#define HISTORY_LEN 100
#define PIPE_BUF 4096

/***********************************************
 * DATA STRUCTURES
 ***********************************************/
typedef struct Command {
  int argc;
  char *name;
  char *argv[MAX_ARGS];
  bool is_out_redirect;
  bool is_in_redirect;
  char *in_file_name;
  char *out_file_name;
  struct Command *next;
} Command;

typedef struct History {
  char *history[HISTORY_LEN];
  int count;
  int current_index;
} History;

/***********************************************
 * TERMINAL MODE MANAGEMENT
 ***********************************************/
void enable_raw_mode(struct termios *orig_termios);
void disable_raw_mode(const struct termios *orig_termios);
void clear_current_line(size_t length);
size_t handle_arrow_key(char *buffer, size_t buffer_size,
                        size_t current_length);

/***********************************************
 * INPUT HANDLING AND PROMPT
 ***********************************************/
void prompt(char cmd[], size_t size);
void read_line(char *buffer, size_t size);

/***********************************************
 * HISTORY MANAGEMENT
 ***********************************************/
void init_history();
void history_add(const char *cmd);
void history_display();
char *read_last_line_from_fd(int fd);
void free_history();

/***********************************************
 * COMMAND PARSING
 ***********************************************/
Command *create_command();
Command *parse_pipeline(char *src);
Command *parse_redirect(char *src);
Command *parse_command(char *src);
void free_commands(Command **head);

/***********************************************
 * COMMAND EXECUTION
 ***********************************************/
void run_commands(const Command *head);
bool handle_builtins(const Command *cmd);
pid_t execute_command(const Command *cmd, int prev_pipe, int pipefd[2]);
void setup_redirections(const Command *cmd);
void setup_pipes(int prev_pipe, int pipefd[2], bool has_next);
bool execute(const Command *cmd);
void try_paths(const char *paths, const Command *cmd);

/***********************************************
 * BUILT-IN COMMANDS
 ***********************************************/
void change_dir(const Command *cmd);
void tree(const char *cwd, size_t level);

/***********************************************
 * STRING UTILITIES
 ***********************************************/
char *strtok_q(char *str, const char *delim, char **saveptr);
char *trim(char *str);
void print_command(const Command *cmd);

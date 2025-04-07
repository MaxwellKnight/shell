#pragma once
#include <stdbool.h>
#include <stdlib.h>
#define MAX_ARGS 20
#define INPUT_LEN 256
#define HISTORY_LEN 100
#define PIPE_BUF 4096

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
  int start;
  int count;
  int index;
} History;

Command *create_command();
Command *parse_pipeline(char *src);
Command *parse_redirect(char *src);
Command *parse_command(char *src);
bool handle_builtins(const Command *cmd);
pid_t execute_command(const Command *cmd, int prev_pipe, int pipefd[2]);
void setup_redirections(const Command *cmd);
void setup_pipes(int prev_pipe, int pipefd[2], bool has_next);
void run_commands(const Command *head);
bool execute(const Command *cmd);
void tree(const char *cwd, size_t level);
void try_paths(const char *paths, const Command *cmd);

// ============== Helper functions ============== //
void prompt(char cmd[], size_t size);
void print_command(const Command *cmd);
void read_line(char *buffer, size_t size);
char *strtok_q(char *str, const char *delim, char **saveptr);
char *trim(char *str);

// ============== Builtin command ============== //
void change_dir(const Command *cmd);
void history_add(const char *cmd);
void history_display();
char *read_last_line_from_fd(int fd);

// ============== Memory ============== //
void free_commands(Command **head);

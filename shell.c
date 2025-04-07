#include "shell.h"
#include "colors.h"
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

extern char **environ;
History cmd_history = {0};

/***********************************************
 * TERMINAL MODE MANAGEMENT
 ***********************************************/

void enable_raw_mode(struct termios *orig_termios) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, orig_termios);
  raw = *orig_termios;

  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_cflag |= (CS8);
  raw.c_oflag &= ~(OPOST);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(const struct termios *orig_termios) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

/***********************************************
 * INPUT HANDLING AND PROMPT
 ***********************************************/

void clear_current_line(size_t length) {
  for (size_t j = 0; j < length; j++) {
    printf("\b \b");
  }
  fflush(stdout);
}

size_t handle_arrow_key(char *buffer, size_t buffer_size,
                        size_t current_length) {
  char seq[2];
  if (read(STDIN_FILENO, &seq[0], 1) != 1)
    return current_length;
  if (read(STDIN_FILENO, &seq[1], 1) != 1)
    return current_length;

  if (seq[0] == '[') {
    if (seq[1] == 'A') { // Up arrow
      if (cmd_history.current_index < cmd_history.count - 1) {
        cmd_history.current_index++;
        if (cmd_history.history[cmd_history.current_index]) {
          clear_current_line(current_length);
          strncpy(buffer, cmd_history.history[cmd_history.current_index],
                  buffer_size - 1);
          buffer[buffer_size - 1] = '\0';
          printf("%s", buffer);
          fflush(stdout);
          return strlen(buffer);
        }
      }
    } else if (seq[1] == 'B') { // Down arrow
      if (cmd_history.current_index > 0) {
        cmd_history.current_index--;
        clear_current_line(current_length);
        strncpy(buffer, cmd_history.history[cmd_history.current_index],
                buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        printf("%s", buffer);
        fflush(stdout);
        return strlen(buffer);
      } else if (cmd_history.current_index == 0) {
        cmd_history.current_index = -1;
        clear_current_line(current_length);
        buffer[0] = '\0';
        fflush(stdout);
        return 0;
      }
    }
  }

  return current_length;
}

void read_line(char *buffer, size_t size) {
  memset(buffer, 0, size);
  struct termios orig_termios;
  enable_raw_mode(&orig_termios);

  size_t i = 0;
  char c;

  while (i < size - 1) {
    ssize_t nread = read(STDIN_FILENO, &c, 1);
    if (nread <= 0)
      continue;

    if (c == '\n' || c == '\r') {
      buffer[i] = '\0';
      break;
    } else if (c == 127 || c == '\b') {
      if (i > 0) {
        i--;
        buffer[i] = '\0';
        printf("\b \b");
      }
    } else if (c == 27) {
      i = handle_arrow_key(buffer, size, i);
    } else if (c >= 32 && c <= 126) {
      buffer[i++] = c;
      buffer[i] = '\0';
      putchar(c);
    }

    fflush(stdout);
  }

  disable_raw_mode(&orig_termios);
  printf("\n");
}

void prompt(char cmd[], size_t size) {
  char cwd[INPUT_LEN];
  char *home = getenv("HOME");
  size_t home_len = 0;

  if (!getcwd(cwd, INPUT_LEN)) {
    strcpy(cwd, "unknown");
  }

  if (home) {
    home_len = strlen(home);
    if (strncmp(cwd, home, home_len) == 0) {
      printf("%s%s~%s%s ", BOLD, BLUE, cwd + home_len, RESET);
    } else {
      printf("%s%s%s%s ", BOLD, BLUE, cwd, RESET);
    }
  } else {
    printf("%s%s%s%s ", BOLD, BLUE, cwd, RESET);
  }

  printf("%s%s|>%s ", BOLD, CYAN, RESET);
  fflush(stdout);
  read_line(cmd, size);
}

/***********************************************
 * HISTORY MANAGEMENT
 ***********************************************/

void init_history() {
  cmd_history.count = 0;
  cmd_history.current_index = -1;
  memset(cmd_history.history, 0, sizeof(cmd_history.history));

  FILE *history_file = fopen("history.txt", "r");
  if (!history_file) {
    return;
  }

  int line_count = 0;
  char buffer[INPUT_LEN];
  while (fgets(buffer, INPUT_LEN, history_file) && line_count < HISTORY_LEN) {
    line_count++;
  }

  rewind(history_file);

  char **lines = (char **)malloc(line_count * sizeof(char *));
  if (!lines) {
    fclose(history_file);
    return;
  }

  int actual_lines = 0;
  while (actual_lines < line_count && fgets(buffer, INPUT_LEN, history_file)) {
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }

    char *tab_pos = strchr(buffer, '\t');
    if (tab_pos) {
      lines[actual_lines++] = strdup(tab_pos + 1);
    }
  }

  fclose(history_file);

  int history_index = 0;
  for (int i = actual_lines - 1; i >= 0 && history_index < HISTORY_LEN; i--) {
    cmd_history.history[history_index++] = lines[i];
  }

  cmd_history.count = history_index;
  free(lines);
}

void history_display() {
  int read_fd = open("history.txt", O_RDONLY);
  char buffer[INPUT_LEN];
  char ch;

  if (read_fd == -1) {
    perror("open");
    return;
  }

  while ((ch = read(read_fd, buffer, INPUT_LEN)) != 0) {
    printf("%s", buffer);
  }

  close(read_fd);
}

void history_add(const char *cmd) {
  if (strlen(cmd) == 0) {
    return;
  }

  if (cmd_history.count > 0 && strcmp(cmd_history.history[0], cmd) == 0) {
    return;
  }

  for (int i = HISTORY_LEN - 1; i > 0; i--) {
    if (cmd_history.history[i - 1]) {
      free(cmd_history.history[i]);
      cmd_history.history[i] = strdup(cmd_history.history[i - 1]);
    }
  }

  free(cmd_history.history[0]);
  cmd_history.history[0] = strdup(cmd);

  if (cmd_history.count < HISTORY_LEN) {
    cmd_history.count++;
  }

  cmd_history.current_index = -1;

  int read_fd = open("history.txt", O_RDONLY);
  size_t line_id = 0;
  char *last = NULL;

  if (read_fd != -1) {
    last = read_last_line_from_fd(read_fd);
    close(read_fd);
  }

  if (last != NULL) {
    char *id = strtok(last, "\t");
    line_id = atoi(id) + 1;
  }
  free(last);

  int write_fd = open("history.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (write_fd != -1) {
    char buffer[INPUT_LEN];
    snprintf(buffer, INPUT_LEN, "%zu\t%s", line_id, cmd);
    write(write_fd, buffer, strlen(buffer));

    if (strlen(cmd) > 0 && cmd[strlen(cmd) - 1] != '\n') {
      write(write_fd, "\n", 1);
    }

    close(write_fd);
  }
}

char *read_last_line_from_fd(int fd) {
  off_t filesize = lseek(fd, 0, SEEK_END);
  if (filesize == -1 || filesize == 0)
    return NULL;

  char ch;
  off_t pos = filesize - 1;
  int len = 0;
  char buffer[INPUT_LEN];

  while (pos >= 0 && len < INPUT_LEN - 1) {
    lseek(fd, pos, SEEK_SET);
    read(fd, &ch, 1);

    if (ch == '\n' && pos != filesize - 1)
      break;

    buffer[len++] = ch;
    pos--;
  }

  buffer[len] = '\0';

  for (int i = 0; i < len / 2; i++) {
    char tmp = buffer[i];
    buffer[i] = buffer[len - 1 - i];
    buffer[len - 1 - i] = tmp;
  }

  return strdup(buffer);
}

void free_history() {
  for (int i = 0; i < cmd_history.count; i++) {
    free(cmd_history.history[i]);
  }
}

/***********************************************
 * COMMAND PARSING
 ***********************************************/

Command *create_command() {
  Command *cmd = (Command *)malloc(sizeof(Command));
  cmd->next = NULL;
  cmd->argc = 0;
  cmd->name = NULL;
  cmd->is_in_redirect = false;
  cmd->is_out_redirect = false;
  cmd->in_file_name = NULL;
  cmd->out_file_name = NULL;
  return cmd;
}

Command *parse_pipeline(char *src) {
  Command *head = NULL;
  Command *tail = NULL;

  char *saveptr;
  char *segment = strtok_q(src, "|", &saveptr);
  while (segment != NULL) {
    Command *cmd = parse_redirect(segment);
    if (head == NULL) {
      head = cmd;
      tail = cmd;
    } else {
      tail->next = cmd;
      tail = cmd;
    }
    segment = strtok_q(NULL, "|", &saveptr);
  }

  return head;
}

Command *parse_redirect(char *src) {
  Command *command = NULL;
  char *saveptr, *segment;
  if (strchr(src, '<')) {
    // Check if the command has redirect input
    char *segment = strtok_q(src, "<", &saveptr);
    if (segment != NULL) {
      command = parse_command(segment);
      command->in_file_name = trim(strtok_q(NULL, "<", &saveptr));
      command->is_in_redirect = true;
      return command;
    }
  }

  if (strchr(src, '>')) {
    // Check if the comman has redirect input
    segment = strtok_q(NULL, ">", &saveptr);
    if (segment != NULL) {
      command = command ? command : parse_command(segment);
      command->out_file_name = trim(strtok_q(NULL, ">", &saveptr));
      command->is_out_redirect = true;
      return command;
    }
  }

  return parse_command(src);
}

Command *parse_command(char *src) {
  Command *cmd = create_command();
  size_t args_count = 0;
  char *token = strtok(src, " \t\r\n");

  while (token != NULL && args_count < MAX_ARGS) {
    cmd->argv[args_count] = token;
    if (args_count == 0) {
      cmd->name = cmd->argv[0];
    }
    args_count++;
    token = strtok(NULL, " \t\r\n");
  }

  cmd->argc = args_count;
  cmd->argv[args_count] = NULL;
  return cmd;
}

void free_commands(Command **head) {
  Command *current = *head;
  while (current) {
    Command *deleted = current;
    current = current->next;
    free(deleted);
  }
}

/***********************************************
 * COMMAND EXECUTION
 ***********************************************/

bool handle_builtins(const Command *cmd) {
  if (strcmp(cmd->name, "exit") == 0) {
    exit(EXIT_SUCCESS);
  } else if (strcmp(cmd->name, "cd") == 0) {
    change_dir(cmd);
    return true;
  } else if (strcmp(cmd->name, "history") == 0) {
    history_display();
    return true;
  } else if (strcmp(cmd->name, "tree") == 0) {
    char cwd[INPUT_LEN] = {0};
    getcwd(cwd, INPUT_LEN);
    tree(cwd, 0);
    return true;
  }
  return false;
}

void run_commands(const Command *head) {
  int prev_pipe_read = -1;
  const Command *current = head;
  pid_t pids[MAX_ARGS];
  int cmd_index = 0;

  while (current) {
    if (handle_builtins(current)) {
      current = current->next;
      continue;
    }

    int pipefd[2];
    bool has_next = current->next != NULL;

    if (has_next && pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

    pids[cmd_index++] = execute_command(current, prev_pipe_read, pipefd);

    if (prev_pipe_read != -1)
      close(prev_pipe_read);
    prev_pipe_read = has_next ? pipefd[0] : -1;

    if (has_next)
      close(pipefd[1]);

    current = current->next;
  }

  for (int i = 0; i < cmd_index; ++i) {
    int status;
    waitpid(pids[i], &status, 0);
  }
}

pid_t execute_command(const Command *cmd, int prev_pipe, int pipefd[2]) {
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    setup_pipes(prev_pipe, pipefd, cmd->next != NULL);
    setup_redirections(cmd);
    execute(cmd);
    exit(EXIT_FAILURE);
  }

  return pid;
}

void setup_redirections(const Command *cmd) {
  if (cmd->is_out_redirect && cmd->out_file_name) {
    int fd = open(cmd->out_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
      perror(cmd->out_file_name);
      exit(EXIT_FAILURE);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
  }

  if (cmd->is_in_redirect && cmd->in_file_name) {
    int fd = open(cmd->in_file_name, O_RDONLY);
    if (fd == -1) {
      perror(cmd->in_file_name);
      exit(EXIT_FAILURE);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
  }
}

void setup_pipes(int prev_pipe_read, int pipefd[2], bool has_next) {
  if (prev_pipe_read != -1) {
    dup2(prev_pipe_read, STDIN_FILENO);
    close(prev_pipe_read);
  }

  if (has_next) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
  }
}

bool execute(const Command *cmd) {
  if (strncmp(".", cmd->name, strlen(".")) == 0) {
    if (execve(cmd->name, cmd->argv, environ) == -1) {
      return false;
    }
  }

  const char *paths = getenv("PATH");
  try_paths(paths, cmd);
  // If code gets here means all paths tried
  printf("ERROR: Command \'%s\' not found\n", cmd->name);
  return false;
}

void try_paths(const char *paths, const Command *cmd) {
  char *all_paths = strdup(paths);
  const char *path = strtok(all_paths, ":");
  while (path != NULL) {
    char full_path[INPUT_LEN];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, cmd->name);

    execve(full_path, cmd->argv, environ);
    path = strtok(NULL, ":");
  }
  free(all_paths);
}

/***********************************************
 * BUILT-IN COMMANDS
 ***********************************************/

void change_dir(const Command *cmd) {
  if (cmd->argc < 2) {
    fprintf(stderr, "cd: missing operand\n");
    return;
  }

  const char *target = cmd->argv[1];

  if (target[0] == '~') {
    const char *home = getenv("HOME");
    if (!home) {
      fprintf(stderr, "cd: HOME not set\n");
      return;
    }

    char path[INPUT_LEN];
    if (strlen(target) > 1 && target[1] == '/') {
      snprintf(path, INPUT_LEN, "%s/%s", home, target + 2);
    } else {
      snprintf(path, INPUT_LEN, "%s", home);
    }

    if (chdir(path) != 0) {
      perror("cd");
    }

  } else if (target[0] == '/') {
    if (chdir(target) != 0) {
      perror("cd");
    }

  } else {
    char cwd[INPUT_LEN];
    if (!getcwd(cwd, sizeof(cwd))) {
      perror("cd: getcwd failed");
      return;
    }

    char path[INPUT_LEN];
    snprintf(path, INPUT_LEN, "%s/%s", cwd, target);

    if (chdir(path) != 0) {
      perror("cd");
    }
  }
}

void tree(const char *cwd, size_t level) {
  DIR *dir = opendir(cwd);
  const struct dirent *direntp;
  if (dir == NULL) {
    return;
  }

  while ((direntp = readdir(dir)) != NULL) {
    const char *name = direntp->d_name;
    for (int i = 0; i < level; i++) {
      printf("│   ");
    }

    printf("├── %s", name);
    if (direntp->d_type == DT_DIR) {
      printf("/\n");
      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        continue;
      }

      char next_dir[INPUT_LEN];
      snprintf(next_dir, sizeof(next_dir), "%s/%s", cwd, name);
      tree(next_dir, level + 1);
    } else {
      printf("\n");
    }
  }

  closedir(dir);
}

/***********************************************
 * STRING UTILITIES
 ***********************************************/

char *strtok_q(char *str, const char *delim, char **saveptr) {
  char *token_start;
  if (str != NULL) {
    *saveptr = str;
  }

  token_start = *saveptr;
  if (token_start == NULL) {
    return NULL;
  }

  bool in_quote = false;
  char quote_char = '\0';

  while (**saveptr) {
    if ((**saveptr == '"' || **saveptr == '\'') &&
        (*saveptr == token_start || *(*saveptr - 1) != '\\')) {
      if (!in_quote) {
        in_quote = true;
        quote_char = **saveptr;
      } else if (**saveptr == quote_char) {
        in_quote = false;
      }
    }

    if (strchr(delim, **saveptr) && !in_quote) {
      **saveptr = '\0';
      (*saveptr)++;
      return token_start;
    }

    (*saveptr)++;
  }

  if (token_start == *saveptr) {
    return NULL;
  } else {
    *saveptr = NULL;
    return token_start;
  }
}

char *trim(char *str) {
  char *end;
  while (isspace((unsigned char)*str))
    str++;

  if (*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  *(end + 1) = '\0';
  return str;
}

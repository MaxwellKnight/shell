#include "../include/shell.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern History cmd_history;
extern char *read_last_line_from_fd(int fd);
extern void init_history(void);
extern void history_add(const char *cmd);
extern void free_history(void);

#define TEST_HISTORY_FILE "test_history.txt"
static void create_test_history_file() {
  int fd = open(TEST_HISTORY_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    perror("Failed to create test history file");
    exit(EXIT_FAILURE);
  }

  const char *content =
    "0\tls -la\n"
    "1\techo hello\n"
    "2\tgrep pattern file.txt\n"
    "3\tcat /etc/passwd\n"
    "4\tps aux\n";

  write(fd, content, strlen(content));
  close(fd);
}

static void cleanup_test_history_file() {
  unlink(TEST_HISTORY_FILE);
}

static void setup_test_env() {
  if (access("history.txt", F_OK) == 0) {
    rename("history.txt", "history.txt.bak");
  }
  symlink(TEST_HISTORY_FILE, "history.txt");
}

static void restore_original_env() {
  unlink("history.txt");
  if (access("history.txt.bak", F_OK) == 0) {
    rename("history.txt.bak", "history.txt");
  }
}

static void test_read_last_line() {
  printf("Testing read_last_line_from_fd...\n");

  create_test_history_file();

  int fd = open(TEST_HISTORY_FILE, O_RDONLY);
  if (fd == -1) {
    perror("Failed to open test history file");
    exit(EXIT_FAILURE);
  }

  char *last_line = read_last_line_from_fd(fd);
  assert(last_line != NULL);
  assert(strcmp(last_line, "4\tps aux") == 0);

  free(last_line);
  close(fd);

  printf("read_last_line_from_fd test passed!\n");
}

static void test_init_history() {
  printf("Testing init_history...\n");

  create_test_history_file();
  setup_test_env();

  init_history();

  assert(cmd_history.count > 0);
  assert(cmd_history.current_index == -1);

  assert(strcmp(cmd_history.history[0], "ps aux") == 0);

  if (cmd_history.count > 1) {
    assert(strcmp(cmd_history.history[1], "cat /etc/passwd") == 0);
  }

  free_history();
  restore_original_env();

  printf("init_history test passed!\n");
}

static void test_history_add() {
  printf("Testing history_add...\n");

  create_test_history_file();
  setup_test_env();

  init_history();

  const char *new_cmd = "pwd";
  history_add(new_cmd);

  assert(strcmp(cmd_history.history[0], "pwd") == 0);

  const char *new_cmd2 = "ls -l";
  history_add(new_cmd2);

  assert(strcmp(cmd_history.history[0], "ls -l") == 0);
  assert(strcmp(cmd_history.history[1], "pwd") == 0);

  history_add("ls -l");
  assert(strcmp(cmd_history.history[0], "ls -l") == 0);
  assert(strcmp(cmd_history.history[1], "pwd") == 0);

  free_history();
  restore_original_env();

  printf("history_add test passed!\n");
}

static void test_arrow_navigation() {
  printf("Testing arrow key navigation...\n");

  create_test_history_file();
  setup_test_env();

  init_history();

  char buffer[INPUT_LEN] = "";
  size_t buffer_size = INPUT_LEN;
  cmd_history.current_index = -1;

  if (cmd_history.current_index < cmd_history.count - 1) {
    cmd_history.current_index++;
    if (cmd_history.history[cmd_history.current_index]) {
      strncpy(buffer, cmd_history.history[cmd_history.current_index], buffer_size - 1);
      buffer[buffer_size - 1] = '\0';
    }
  }

  assert(strcmp(buffer, cmd_history.history[0]) == 0);

  if (cmd_history.current_index < cmd_history.count - 1) {
    cmd_history.current_index++;
    if (cmd_history.history[cmd_history.current_index]) {
      strncpy(buffer, cmd_history.history[cmd_history.current_index], buffer_size - 1);
      buffer[buffer_size - 1] = '\0';
    }
  }

  assert(strcmp(buffer, cmd_history.history[1]) == 0);

  if (cmd_history.current_index > 0) {
    cmd_history.current_index--;
    strncpy(buffer, cmd_history.history[cmd_history.current_index], buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
  }

  assert(strcmp(buffer, cmd_history.history[0]) == 0);

  free_history();
  restore_original_env();

  printf("Arrow key navigation test passed!\n");
}

int main() {
  printf("Running history management tests...\n");

  test_read_last_line();
  test_init_history();
  test_history_add();
  test_arrow_navigation();

  cleanup_test_history_file();

  printf("All history tests passed!\n");
  return 0;
}

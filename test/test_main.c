#include "shell.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern History cmd_history;

void mock_read_line(char *buffer, size_t size, const char *input) {
  strncpy(buffer, input, size - 1);
  buffer[size - 1] = '\0';
}

void test_create_command() {
  printf("Testing create_command...\n");
  Command *cmd = create_command();

  assert(cmd != NULL);
  assert(cmd->next == NULL);
  assert(cmd->argc == 0);
  assert(cmd->name == NULL);
  assert(cmd->is_in_redirect == false);
  assert(cmd->is_out_redirect == false);
  assert(cmd->in_file_name == NULL);
  assert(cmd->out_file_name == NULL);

  free(cmd);
  printf("create_command test passed!\n");
}

void test_builtin_detection() {
  printf("Testing builtin command detection...\n");

  Command *exit_cmd = create_command();
  exit_cmd->name = "exit";
  exit_cmd->argv[0] = "exit";
  exit_cmd->argc = 1;

  Command *cd_cmd = create_command();
  cd_cmd->name = "cd";
  cd_cmd->argv[0] = "cd";
  cd_cmd->argv[1] = "..";
  cd_cmd->argc = 2;

  Command *history_cmd = create_command();
  history_cmd->name = "history";
  history_cmd->argv[0] = "history";
  history_cmd->argc = 1;

  Command *tree_cmd = create_command();
  tree_cmd->name = "tree";
  tree_cmd->argv[0] = "tree";
  tree_cmd->argc = 1;

  Command *ls_cmd = create_command();
  ls_cmd->name = "ls";
  ls_cmd->argv[0] = "ls";
  ls_cmd->argc = 1;

  assert(handle_builtins(exit_cmd) == true);  // We won't actually exit
  assert(handle_builtins(cd_cmd) == true);
  assert(handle_builtins(history_cmd) == true);
  assert(handle_builtins(tree_cmd) == true);
  assert(handle_builtins(ls_cmd) == false);

  free(exit_cmd);
  free(cd_cmd);
  free(history_cmd);
  free(tree_cmd);
  free(ls_cmd);

  printf("Builtin command detection test passed!\n");
}

void test_string_utils() {
  printf("Testing string utilities...\n");

  char str1[] = "  hello world  ";
  char *trimmed = trim(str1);
  assert(strcmp(trimmed, "hello world") == 0);

  char str2[] = "no_trim_needed";
  trimmed = trim(str2);
  assert(strcmp(trimmed, "no_trim_needed") == 0);

  char str3[] = "\t\nwhitespace\t\n";
  trimmed = trim(str3);
  assert(strcmp(trimmed, "whitespace") == 0);

  char str4[] = "cmd1 | cmd2 | \"cmd3 | with pipe\"";
  char *saveptr;
  char *token = strtok_q(str4, "|", &saveptr);
  assert(strcmp(token, "cmd1 ") == 0);

  token = strtok_q(NULL, "|", &saveptr);
  assert(strcmp(token, " cmd2 ") == 0);

  token = strtok_q(NULL, "|", &saveptr);
  assert(strcmp(token, " \"cmd3 | with pipe\"") == 0);

  printf("String utilities tests passed!\n");
}

int main() {
  printf("Running shell main tests...\n");

  test_create_command();
  test_builtin_detection();
  test_string_utils();

  printf("All main tests passed!\n");
  return 0;
}

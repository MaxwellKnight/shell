#include "shell.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External declarations of functions from shell.c
extern History cmd_history;
extern Command *create_command(void);
extern Command *parse_command(char *src);
extern Command *parse_redirect(char *src);
extern Command *parse_pipeline(char *src);
extern void free_commands(Command **head);
extern char *trim(char *str);
extern char *strtok_q(char *str, const char *delim, char **saveptr);

// Helper function to compare commands
static void assert_command_equals(Command *cmd, const char *name, int argc,
                          bool is_in_redirect, bool is_out_redirect,
                          const char *in_file, const char *out_file) {
  assert(cmd != NULL);
  assert(strcmp(cmd->name, name) == 0);
  assert(cmd->argc == argc);
  assert(cmd->is_in_redirect == is_in_redirect);
  assert(cmd->is_out_redirect == is_out_redirect);

  if (in_file) {
    assert(cmd->in_file_name != NULL);
    assert(strcmp(cmd->in_file_name, in_file) == 0);
  } else {
    assert(cmd->in_file_name == NULL);
  }

  if (out_file) {
    assert(cmd->out_file_name != NULL);
    assert(strcmp(cmd->out_file_name, out_file) == 0);
  } else {
    assert(cmd->out_file_name == NULL);
  }
}

// Test simple command parsing
static void test_parse_command() {
  printf("Testing parse_command...\n");

  char cmd_str1[] = "ls -la /home";
  Command *cmd1 = parse_command(cmd_str1);
  assert_command_equals(cmd1, "ls", 3, false, false, NULL, NULL);
  assert(strcmp(cmd1->argv[0], "ls") == 0);
  assert(strcmp(cmd1->argv[1], "-la") == 0);
  assert(strcmp(cmd1->argv[2], "/home") == 0);
  assert(cmd1->argv[3] == NULL);
  free(cmd1);

  char cmd_str2[] = "grep pattern";
  Command *cmd2 = parse_command(cmd_str2);
  assert_command_equals(cmd2, "grep", 2, false, false, NULL, NULL);
  assert(strcmp(cmd2->argv[0], "grep") == 0);
  assert(strcmp(cmd2->argv[1], "pattern") == 0);
  assert(cmd2->argv[2] == NULL);
  free(cmd2);

  char cmd_str3[] = "echo";
  Command *cmd3 = parse_command(cmd_str3);
  assert_command_equals(cmd3, "echo", 1, false, false, NULL, NULL);
  assert(strcmp(cmd3->argv[0], "echo") == 0);
  assert(cmd3->argv[1] == NULL);
  free(cmd3);

  printf("parse_command tests passed!\n");
}

// Test redirection parsing
static void test_parse_redirect() {
  printf("Testing parse_redirect...\n");

  char cmd_str1[] = "ls > output.txt";
  Command *cmd1 = parse_redirect(cmd_str1);
  assert_command_equals(cmd1, "ls", 1, false, true, NULL, "output.txt");
  free(cmd1);

  char cmd_str2[] = "cat < input.txt";
  Command *cmd2 = parse_redirect(cmd_str2);
  assert_command_equals(cmd2, "cat", 1, true, false, "input.txt", NULL);
  free(cmd2);

  char cmd_str3[] = "grep pattern < input.txt > output.txt";
  Command *cmd3 = parse_redirect(cmd_str3);
  assert_command_equals(cmd3, "grep", 2, true, true, "input.txt", "output.txt");
  free(cmd3);

  char cmd_str4[] = "echo hello";
  Command *cmd4 = parse_redirect(cmd_str4);
  assert_command_equals(cmd4, "echo", 2, false, false, NULL, NULL);
  free(cmd4);

  printf("parse_redirect tests passed!\n");
}

static void test_parse_pipeline() {
  printf("Testing parse_pipeline...\n");

  char cmd_str[] = "ls -l | grep txt | wc -l";
  Command *head = parse_pipeline(cmd_str);

  assert_command_equals(head, "ls", 2, false, false, NULL, NULL);
  assert(strcmp(head->argv[0], "ls") == 0);
  assert(strcmp(head->argv[1], "-l") == 0);

  Command *cmd2 = head->next;
  assert_command_equals(cmd2, "grep", 2, false, false, NULL, NULL);
  assert(strcmp(cmd2->argv[0], "grep") == 0);
  assert(strcmp(cmd2->argv[1], "txt") == 0);

  Command *cmd3 = cmd2->next;
  assert_command_equals(cmd3, "wc", 2, false, false, NULL, NULL);
  assert(strcmp(cmd3->argv[0], "wc") == 0);
  assert(strcmp(cmd3->argv[1], "-l") == 0);

  assert(cmd3->next == NULL);
  free_commands(&head);

  char cmd_str2[] = "cat < input.txt | grep pattern | sort > output.txt";
  Command *head2 = parse_pipeline(cmd_str2);

  assert_command_equals(head2, "cat", 1, true, false, "input.txt", NULL);

  Command *cmd2_2 = head2->next;
  assert_command_equals(cmd2_2, "grep", 2, false, false, NULL, NULL);

  Command *cmd3_2 = cmd2_2->next;
  assert_command_equals(cmd3_2, "sort", 1, false, true, NULL, "output.txt");

  free_commands(&head2);
  printf("parse_pipeline tests passed!\n");
}

int main() {
  printf("Running command parsing tests...\n");

  test_parse_command();
  test_parse_redirect();
  test_parse_pipeline();

  printf("All parsing tests passed!\n");
  return 0;
}

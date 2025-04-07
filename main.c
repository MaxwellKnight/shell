#include "shell.h"
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char cmd[INPUT_LEN];
  while (true) {
    prompt(cmd, INPUT_LEN);
    history_add(cmd);
    Command *commands = parse_pipeline(cmd);
    run_commands(commands);
    free_commands(&commands);
  }
}

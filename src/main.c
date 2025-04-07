#include "shell.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  init_history();
  char cmd[INPUT_LEN];

  while (true) {
    prompt(cmd, INPUT_LEN);
    if (strlen(cmd) > 0) {
      history_add(cmd);
      Command *commands = parse_pipeline(cmd);
      run_commands(commands);
      free_commands(&commands);
    }
  }

  free_history();
}

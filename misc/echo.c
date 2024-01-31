#include <unistd.h>
#include <stdio.h>

#include "../src/term.h"

int main(void) {
  term_init();

  char input;
  while (read(0, &input, 1)) {
    printf("%c: %d\n", input, input);
    if (input == 3)
      break;
  }

  term_reset();
  return 0;
}

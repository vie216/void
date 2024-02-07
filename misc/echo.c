#include <stdio.h>

#include "../src/term.h"
#include "../src/defs.h"

int main(void) {
  term_init();

  u8 input;
  for (;;) {
    input = getchar();
    printf("%c:%d:%p:%b\n", input,
           input, (void *) (u64) input,
           input);
    if (input == 3 || input == (u8) EOF)
      break;
  }

  term_reset();
  return 0;
}

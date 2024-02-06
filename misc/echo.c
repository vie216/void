#include <unistd.h>
#include <stdio.h>

#include "../src/term.h"
#include "../src/defs.h"

int main(void) {
  term_init();

  u8 input;
  while (read(0, &input, 1)) {
    printf("%c:%d:%p:%b\n", input,
           input, (void *) (u64) input,
           input);
    if (input == 3)
      break;
  }

  term_reset();
  return 0;
}

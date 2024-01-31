#include <termios.h>
#include <stdio.h>

static struct termios old;

void term_init(void) {
  tcgetattr(0, &old);
  struct termios temp = old;
  temp.c_lflag = ~ICANON | ~ECHO;
  tcsetattr(0, TCSANOW, &temp);
}

void term_reset(void) {
  tcsetattr(0, TCSANOW, &old);
}

void term_clear(void) {
  fputs("\033[H\033[2J", stdout);
}

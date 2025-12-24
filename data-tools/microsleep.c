#include <unistd.h>
#include <stdlib.h>


int main (int argc, char* argv[]) {
  usleep (atoi (argv[1]));
}

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>


int main (int argc, char* argv[]) {
  FILE* fp;
  unsigned char buffer[256];
  unsigned char*c;
  unsigned int temp;
  unsigned int values[9];
  int i = 0;

  if ((2 != argc) || (NULL == argv[1])) {
    fprintf (stderr, "usage %s filename\n", argv[0]);
    exit (-1);
  }
  
  if (NULL == (fp = fopen (argv[1], "r"))) {
    fprintf (stderr, "cannot open %s filename\n", argv[0]);
    exit (-1);
  }
  
  bzero ((char*) values, sizeof (values));

  while (NULL != fgets (buffer, sizeof (buffer), fp)) {
    c = buffer;
    switch (*c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      
      // e.g,: 0:557234,8000B000,0,0,0,0,0,0,0,0
      // or: 0:740770,8000B000,2,26,19,158,19,159,19,158
      // or: 1:350704,,,,,,,,,158

      i = 0;

      // Move to the first comman
      while (('\0' != *c) && (',' != *c)) {
	c++;
      }

      c++;

      if (',' == *(c)) {
	++i;
      } else {
	sscanf (c, "%X[^,]", &temp);
	values[i++] = temp;
	while (('\0' != *c) && (',' != *c)) {
	  c++;
	}
      }

      c++;

      while (i < 9) {
	if (',' == *(c)) {
	  ++i;
	} else {
	  sscanf (c, "%d[^,]", &temp);
	  values[i++] = temp;
	  while (('\0' != *c) && (',' != *c)) {
	    c++;
	  }
	}

	c++;
      }

      printf ("%X#", values[0]);
      for (i = 1; i < 9; i++) {
	printf ("%02x", values[i]);
      }
      
      printf ("\n");
      
      break;

    default:
      continue;
    }
  }

  fclose (fp);
}


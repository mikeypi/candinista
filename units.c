#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "units.h"


char*
str_from_unit_enum (unit_type unit) {
  switch (unit) {
  case CELSIUS: return ("°C");
  case FAHRENHEIT: return ("°F");
  case BAR: return ("bar");
  case PSI: return ("psi");
  default: return ("NONE");
  }
}

unit_type
enum_from_unit_str (char* temp) {
  char buffer[80];
  int i;
  for (i = 0; i < strlen (temp); i++) {
    buffer[i] = tolower (temp[i]);
  }

  buffer[i] = '\0';

  if ((0 == strcmp (buffer, "celsius")) || (0 == strcmp (buffer, "c"))) { fprintf (stderr, "returning CELSIUS\n"); return (CELSIUS); }
  if ((0 == strcmp (buffer, "fahrenheit")) || (0 == strcmp (buffer, "f"))) {  fprintf (stderr, "returning FAHRENHEIT\n");return (FAHRENHEIT); }
  if (0 == strcmp (buffer, "bar")) {  fprintf (stderr, "returning BAR\n");return (BAR); }
  if (0 == strcmp (buffer, "psi")) {  fprintf (stderr, "returning PSI\n");return (PSI); }
  if (0 == strcmp (buffer, "none")) {  fprintf (stderr, "returning NONE\n");return (NONE); }
  
  fprintf (stderr, "unknown unit type\n");
  return (NONE);
}



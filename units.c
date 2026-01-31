#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "units.h"
#include "candinista.h"

char*
str_from_unit_enum (const unit_type unit) {
  switch (unit) {
  case CELSIUS: return ("°C");
  case FAHRENHEIT: return ("°F");
  case BAR: return ("bar");
  case PSI: return ("psi");
  case KPA: return ("kPa");
  default: return ("NONE");
  }
}

unit_type
enum_from_unit_str (const char* temp) {
  char buffer[80];
  int i;
  for (i = 0; i < strlen (temp); i++) {
    buffer[i] = tolower (temp[i]);
  }

  buffer[i] = '\0';
  if ((0 == strcmp (buffer, "celsius"))
      || (0 == strcmp (buffer, "c"))
      || (0 == strcmp (buffer, "°c"))
      ) { return (CELSIUS); }
  if ((0 == strcmp (buffer, "fahrenheit"))
      || (0 == strcmp (buffer, "f"))
      || (0 == strcmp (buffer, "°f"))
      ) { return (FAHRENHEIT); }
  if (0 == strcmp (buffer, "bar"))  {  return (BAR); }
  if (0 == strcmp (buffer, "kpa"))  {  return (KPA); }
  if (0 == strcmp (buffer, "psi"))  {  return (PSI); }
  if (0 == strcmp (buffer, "none")) {  return (NONE); }
  
  fprintf (stderr, "unknown unit type\n");
  return (NONE);
}

double
convert_units (const double temp, unit_type to) {
  if (0 != raw_output) {
    return (temp);
  }
  switch (to) {
  case FAHRENHEIT:
    return ((temp * 9.0 / 5.0) + 32.0);

  case PSI:
    return (temp * 14.503773773);

  case KPA:
    return (temp * 100);

  default:
    return (temp);
  }
}

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
  case MPH: return ("mph");
  case KPH: return ("kph");
  case FOOT: return ("foot");
  case METER: return ("meter");
  default: return ("UNKNOWN_UNIT");
  }
}

unit_type
enum_from_unit_str (const char* temp) {
  char buffer[80];
  size_t i;
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
  if (0 == strcmp (buffer, "mph"))  {  return (MPH); }
  if (0 == strcmp (buffer, "kph"))  {  return (KPH); }
  if (0 == strcmp (buffer, "foot"))  {  return (FOOT); }
  if (0 == strcmp (buffer, "meter"))  {  return (METER); }
  if (0 == strcmp (buffer, "none")) {  return (UNKNOWN_UNIT); }
  
  fprintf (stderr, "unknown unit type\n");
  return (UNKNOWN_UNIT);
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

  case MPH:
    return (temp * 0.621371);

  case FOOT:
    return (temp * 3.28084);
    
  default:
    return (temp);
  }
}

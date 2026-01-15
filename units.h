#ifndef UNITS_H
#define UNITS_H

typedef enum {FAHRENHEIT, CELSIUS, BAR, PSI, NONE} unit_type;

extern char* str_from_unit_enum (unit_type unit);
extern unit_type enum_from_unit_str (char* temp);

#endif

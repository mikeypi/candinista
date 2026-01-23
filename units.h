#ifndef UNITS_H
#define UNITS_H

typedef enum {FAHRENHEIT, CELSIUS, BAR, PSI, KPA, NONE} unit_type;

extern char* str_from_unit_enum (const unit_type unit);
extern unit_type enum_from_unit_str (const char* temp);
extern double convert_units (const double temp, unit_type to);

#endif

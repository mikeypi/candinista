
/*
 * Copyright (c) 2024, Joseph Hollinger
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <sys/file.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <assert.h>
#include <linux/can.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

static const char*
str_from_unit_enum (unit_type e) {
  switch (e) {
  case CELSIUS: return ("celsius");
  case FAHRENHEIT: return ("fahrenheit");
  case BAR: return ("bar");
  case PSI: return ("psi");
  case NONE: return ("none");
  default: return ("error, unknown enum");
  }
}


static const char*
ntabs (int count) {
  switch (count)
    {
    case 5: return ("\t\t\t\t\t");
    case 4: return ("\t\t\t\t");
    case 3: return ("\t\t\t");
    case 2: return ("\t\t");
    case 1: return ("\t");
    default: return (" ");
    }
}

#define TAB_LEVEL(X) tabs = ntabs (X);

static char printable_string_buffer[80];

static const char*
printable_string (char* c) {
  char *x = printable_string_buffer;
  strcpy (printable_string_buffer, c);
  while ('\0' != *x) {
    if ('\n'== *x) {
      *x = ' ';
    }
    
    x++;
  }

  return (printable_string_buffer);
}


static void
print_output (output_descriptor* p, int indent, FILE* fp) {
  const char* tabs;
  TAB_LEVEL(indent);
  fprintf (fp, "\nOutput = %s{\n", tabs);
  TAB_LEVEL(indent + 1);
  fprintf (fp, "%s\"name\": \"%s\",\n", tabs, printable_string (p -> label));
  fprintf (fp, "%s\"min\": %f,\n", tabs, p -> min);
  fprintf (fp, "%s\"max\": %f,\n", tabs, p -> max);
  fprintf (fp, "%s\"box number\": %d,\n", tabs, p -> box_number);
  fprintf (fp, "%s\"units\": \"%s\",\n", tabs, str_from_unit_enum (p -> units));
  fprintf (fp, "%s\"output_format\": \"%s\"\n", tabs, p -> output_format);
  TAB_LEVEL(indent);
  fprintf (fp, "%s}\n\n", tabs);
}


static void
print_sensor (sensor_descriptor* p, int indent, FILE* fp) {
  const char* tabs;
  TAB_LEVEL(indent);
  fprintf (fp, "\nSensor = %s{\n", tabs);
  TAB_LEVEL(indent + 1);
  fprintf (fp, "%s\"name\": \"%s\",\n", tabs, p -> name);
  fprintf (fp, "%s\"can data offset\": \"%x\",\n", tabs, p -> can_data_offset);
  fprintf (fp, "%s\"can data width\": \"%x\",\n", tabs, p -> can_data_width);
  fprintf (fp, "%s\"can id\": \"%x\",\n", tabs, p -> can_id);
    
  fprintf (fp, "%s\"offset\": \"%f\",\n", tabs, p -> offset);
  fprintf (fp, "%s\"number of interpolation points\": \"%d\",\n", tabs, p -> number_of_interpolation_points);
  
  fprintf (fp, "%s\"x values\": [", tabs);

  int j = 0;
  while (j < p -> number_of_interpolation_points) {

    fprintf (fp, "%.1f", p -> x_values[j]);

    if (j != p -> number_of_interpolation_points - 1) {
      fprintf (fp, ", ");
    }
      
    if ((0 != j) && (0 == (j % 8))) {
      fprintf (fp, "\n%s", tabs);
    }

    j++;
  }

  fprintf (fp, "],\n");

  fprintf (fp, "%s\"y values\": [", tabs);

  j = 0;
  while (j < p -> number_of_interpolation_points) {
    fprintf (fp, "%.1f", p -> y_values[j]);

    if (j != p -> number_of_interpolation_points - 1) {
      fprintf (fp, ", ");
    }
      
    if ((0 != j) && (0 == (j % 8))) {
      fprintf (fp, "\n%s", tabs);
    }

    j++;
  }

  fprintf (fp, "]\n");

  TAB_LEVEL(indent);
  fprintf (fp, "%s}", tabs);
}


void	      
print_config (FILE* fp) {
  for (int i = 0; i < sensor_count; i++) {
    print_sensor (&sensor_descriptors[i], 0, fp);
    print_output (sensor_descriptors[i].output_descriptor, 0, fp);
  }
}


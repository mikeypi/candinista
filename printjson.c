#include <stdio.h>
#include <sys/file.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

extern int frame_count;

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

#define TAB_LEVEL(X) tabs = ntabs (X);
  
static void
print_frame (frame_descriptor* p, int indent) {
  const char* tabs;

  TAB_LEVEL(indent);
  fprintf (stderr, "\n%s{\n", tabs);
  TAB_LEVEL(indent + 1); 
  fprintf (stderr, "%s\"name\": \"%s\",\n", tabs, p -> name);
  fprintf (stderr, "%s\"id\": \"0x%X\",\n", tabs, p -> id);

  fprintf (stderr, "%s\"field offsets\": [", tabs);
  int j = 0;
  while (j < p -> field_count) {
    fprintf (stderr, "%d", p -> field_offsets[j]);
    if (j != p -> field_count - 1) {
      fprintf (stderr, ", ");
    }

    j++;
  }

  fprintf (stderr, "],\n");

  fprintf (stderr, "%s\"field sizes\": [", tabs);
  j = 0;
  while (j < p -> field_count) {
    fprintf (stderr, "%d", p -> field_sizes[j]);
    if (j != p -> field_count - 1) {
      fprintf (stderr, ", ");
    }

    j++;
  }

  fprintf (stderr, "]\n");
  
  TAB_LEVEL(indent);
  fprintf (stderr, "%s}", tabs);
}


static void
print_sensor (sensor_descriptor* p, int indent) {
  const char* tabs;
  TAB_LEVEL(indent);
  fprintf (stderr, "\n%s{\n", tabs);
  TAB_LEVEL(indent + 1);
  fprintf (stderr, "%s\"name\": \"%s\",\n", tabs, p -> name);
  fprintf (stderr, "%s\"offset\": \"%d\",\n", tabs, p -> offset);
  fprintf (stderr, "%s\"number of interpolation points\": \"%d\",\n", tabs, p -> number_of_interpolation_points);
  
  fprintf (stderr, "%s\"x values\": [", tabs);

  int j = 0;
  while (j < p -> number_of_interpolation_points) {

    fprintf (stderr, "%.1f", p -> x_values[j]);

    if (j != p -> number_of_interpolation_points - 1) {
      fprintf (stderr, ", ");
    }
      
    if ((0 != j) && (0 == (j % 8))) {
      fprintf (stderr, "\n%s", tabs);
    }

    j++;
  }

  fprintf (stderr, "],\n");

  fprintf (stderr, "%s\"y values\": [", tabs);

  j = 0;
  while (j < p -> number_of_interpolation_points) {
    fprintf (stderr, "%.1f", p -> y_values[j]);

    if (j != p -> number_of_interpolation_points - 1) {
      fprintf (stderr, ", ");
    }
      
    if ((0 != j) && (0 == (j % 8))) {
      fprintf (stderr, "\n%s", tabs);
    }

    j++;
  }

  fprintf (stderr, "]\n");

  TAB_LEVEL(indent);
  fprintf (stderr, "%s}", tabs);
}


static void
print_output (output_descriptor* p, int indent) {
  const char* tabs;
  TAB_LEVEL(indent);
  fprintf (stderr, "\n%s{\n", tabs);
  TAB_LEVEL(indent + 1);
  fprintf (stderr, "%s\"name\": \"%s\",\n", tabs, printable_string (p -> label));
  fprintf (stderr, "%s\"min\": %d,\n", tabs, p -> min);
  fprintf (stderr, "%s\"max\": %d,\n", tabs, p -> max);
  fprintf (stderr, "%s\"box number\": %d,\n", tabs, p -> box_number);
  fprintf (stderr, "%s\"units\": \"%s\",\n", tabs, str_from_unit_enum (p -> units));
  fprintf (stderr, "%s\"output_format\": \"%s\"\n", tabs, p -> output_format);
  TAB_LEVEL(indent);
  fprintf (stderr, "%s}", tabs);
}


void
print_config () {
  const char* tabs;
  fprintf (stderr, "{\n");
  TAB_LEVEL(1);
  fprintf (stderr, "\n%s\"top level descriptors\":\n", tabs);
  fprintf (stderr, "\n%s[\n", tabs);

  TAB_LEVEL(2);

  for (int i = 0; i < frame_count; i++) {
    fprintf (stderr, "%s{\n", tabs);

    TAB_LEVEL(3);
    fprintf (stderr, "%s\"frame\":", tabs);
    print_frame (top_level_descriptors[i].frame_descriptor, 3);
    fprintf (stderr, ",\n");

    fprintf (stderr, "%s\"sensor inputs\":", tabs);
    fprintf (stderr, "[", tabs);

    int j = 0;
    while (j < top_level_descriptors[i].sensor_descriptor_count) {
      print_sensor (top_level_descriptors[i].sensor_descriptors[j], 4);
      if (j != top_level_descriptors[i].sensor_descriptor_count - 1) {
	fprintf (stderr, ",");
      }
      j++;
    }

    fprintf (stderr, "\n%s],\n", tabs);
    
    fprintf (stderr, "%s\"sensor ouputs\":", tabs);
    fprintf (stderr, "[", tabs);

    j = 0;
    while (j < top_level_descriptors[i].output_descriptor_count) {
      print_output (top_level_descriptors[i].output_descriptors[j], 4);
      if (j != top_level_descriptors[i].output_descriptor_count - 1) {
	fprintf (stderr, ",");
      }
      j++;
    }

    fprintf (stderr, "\n%s]\n", tabs);
    TAB_LEVEL(2);

    fprintf (stderr, "\n%s}", tabs);
    if (i != frame_count - 1) {
      fprintf (stderr, ",");
    }

    fprintf (stderr, "\n");
    
  }
  
  tabs = ntabs (1);
  fprintf (stderr, "\n%s]\n", tabs);
  fprintf (stderr, "}\n");
}



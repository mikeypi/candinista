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
#include <ctype.h>
#include <json.h>
#include <assert.h>
#include <sys/param.h>
#include <linux/can.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

sensor_descriptor* sensor_descriptors;
output_descriptor* output_descriptors;
int sensor_count;
int output_count;


static json_object*
get_object_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if (NULL == x) {
    if (0 != warn) {
      fprintf (stderr, "required field %s not found\n", field);
    }
    return (NULL);
  }
 
  return (x);
}


static char*
get_string_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if (NULL == x) {
    if (0 != warn) {
      fprintf (stderr, "required field %s not found\n", field);
    }
    return (NULL);
  }

  const char* t = json_object_get_string (x);

  if (NULL == t) {
    return (NULL);
  }
  
  char* temp = (char*) calloc (1 + strlen (t), sizeof (char));

  if (NULL == temp) {
    return (NULL);
  }
  
  return (strcpy (temp, t));
}


static int
get_int_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if (NULL == x)
    if (0 != warn) {
      return (-1);
    }
  
  return (json_object_get_int (x));
}


static float
get_float_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if (NULL == x)
    if (0 != warn) {
      return (-1.0);
    }
  
  return ((float) json_object_get_double (x));
}


#ifdef JJHJK
static sensor_descriptor*
sensor_descriptor_by_name (const char* name) {
  int i;
  for (i = 0; i < sensor_count; i++) {
    if (0 == strcmp (sensor_descriptors[i].name, name)) {
      return (&sensor_descriptors[i]);
    }
  }

  fprintf (stderr, "required sensor descriptor with name %s not found\n", name);
  return (NULL);
}
#endif


output_descriptor*
output_descriptor_by_name (const char* name) {
  int i;
  for (i = 0; i < output_count; i++) {
    if (0 == strcmp (output_descriptors[i].name, name)) {
      return (&output_descriptors[i]);
    }
  }

  fprintf (stderr, "required output descriptor with name %s not found\n", name);
  return (NULL);
}


output_descriptor*
output_descriptor_by_id (const int x) {
  int i;
  for (i = 0; i < output_count; i++) {
    if (x == output_descriptors[i].id) {
      return (&output_descriptors[i]);
    }
  }

  fprintf (stderr, "required output descriptor with id %d not found\n", x);
  return (NULL);
}


static unit_type
enum_from_unit_str (const char* temp) {
  char buffer[80];
  int i;
  for (i = 0; i < strlen (temp); i++) {
    buffer[i] = tolower (temp[i]);
  }

  buffer[i] = '\0';

  if ((0 == strcmp (buffer, "celsius")) || (0 == strcmp (buffer, "c"))) {
    return (CELSIUS);
  }
  if ((0 == strcmp (buffer, "fahrenheit")) || (0 == strcmp (buffer, "f"))) {
    return (FAHRENHEIT);
  }
  if (0 == strcmp (buffer, "bar")) {
    return (BAR);
  }
  if (0 == strcmp (buffer, "psi")) {
    return (PSI);
  }
  if (0 == strcmp (buffer, "none")) {
    return (NONE);
  }
  
  fprintf (stderr, "unknown unit type\n");
  return (NONE);
}

  
static int
add_outputs_from_json (json_object* root) {
  json_object* outputs = get_object_from_json (root, "outputs", 1);
  int noutputs = json_object_array_length (outputs);
  output_count = 0;
  output_descriptors = (output_descriptor*) calloc (noutputs * sizeof (output_descriptor), sizeof (char));
  
  for (int i = 0; i < noutputs; i++) {
    json_object* e = json_object_array_get_idx (outputs, i);
    const char* temp;
    
    output_count++;

    output_descriptors[i].id = get_int_from_json (e, "id", 1);
    output_descriptors[i].label = get_string_from_json (e, "label", 1);
    output_descriptors[i].legend = get_string_from_json (e, "legend", 0);

    output_descriptors[i].row = get_int_from_json (e, "row", 1);
    output_descriptors[i].column = get_int_from_json (e, "column", 1);
    output_descriptors[i].min = get_float_from_json (e, "minimum value", 1);
    output_descriptors[i].max = get_float_from_json (e, "maximum value", 1);
    output_descriptors[i].low_warn = get_float_from_json (e, "low warn level", 0);
    output_descriptors[i].high_warn = get_float_from_json (e, "high warn level", 0);
    output_descriptors[i].offset = get_float_from_json (e, "offset", 0);

    temp = get_string_from_json (e, "units", 1);
    output_descriptors[i].units = enum_from_unit_str (temp);

#ifdef DEBUG
    printf ("added output %s\n", output_descriptors[i].name);
#endif
}

#ifdef DEBUG
  printf ("added %d outputs\n", output_count);
#endif
  return (output_count);
}


static int
add_sensors_from_json (json_object* root) {
  json_object* sensors = get_object_from_json (root, "sensors", 1);  
  int nsensors = json_object_array_length (sensors);
  sensor_count = 0;
  
  sensor_descriptors = (sensor_descriptor*) calloc (nsensors * sizeof (sensor_descriptor), sizeof (char));

  for (int i = 0; i < nsensors; i++) {
    const char* temp;
    json_object* e = json_object_array_get_idx (sensors, i);

    sensor_descriptors[i].name = get_string_from_json (e, "name", 1);

    /*
     * JSON requires all numbers be in hex. That's a little unnatural for CAN IDs, so those are strings in the
     * config file (e.g., 0x0003). This code converts them from strings to numbers.
     */
    temp = get_string_from_json (e, "can id", 1);
    if (NULL == temp) {
      fprintf (stderr, "required field can id for sensor descriptor %s not found,skipping\n",
	       sensor_descriptors[i].name);
      continue;
    }

    sensor_count++;

    sensor_descriptors[i].can_id = strtol (temp, NULL, 16);
    sensor_descriptors[i].can_data_offset = get_int_from_json (e, "can data offset", 1);
    sensor_descriptors[i].can_data_width = get_int_from_json (e, "can data width", 1);

    int j = get_int_from_json (e, "output id", 1);

    if (j < 0) {
      sensor_descriptors[i].output_descriptor = NULL;
    }
    else {
      sensor_descriptors[i].output_descriptor = output_descriptor_by_id (j);
    }
      
    json_object* x_values = get_object_from_json (e, "x values", 1);
    int x_value_count = json_object_array_length (x_values);
    
    json_object* y_values = get_object_from_json (e, "y values", 1);
    int y_value_count = json_object_array_length (y_values);

    assert (x_value_count == y_value_count);
    
    sensor_descriptors[i].number_of_interpolation_points = x_value_count;

    sensor_descriptors[i].x_values = (float*) calloc (x_value_count, sizeof (float));
    sensor_descriptors[i].y_values = (float*) calloc (y_value_count, sizeof (float));
    
    for (int j = 0; j < x_value_count; j++) {
      sensor_descriptors[i].x_values[j] = json_object_get_int (json_object_array_get_idx (x_values, j));
      sensor_descriptors[i].y_values[j] = json_object_get_int (json_object_array_get_idx (y_values, j));
    }

#ifndef TEST
    interpolation_array_sort (&sensor_descriptors[i]);
#endif

#ifdef DEBUG
    printf ("added sensor %s with can id %x at offset %d and width %d with %d interpolation points\n",
	    sensor_descriptors[i].name,
	    sensor_descriptors[i].can_id,
	    sensor_descriptors[i].can_data_offset,
	    sensor_descriptors[i].can_data_width,
	    y_value_count);
#endif
  }

  printf ("added %d sensors\n", sensor_count);
  return (sensor_count);
}


void
read_config_from_json (void)
{
#ifdef DEBUG
  config_file_name = "/home/joe/candinista/config.json";
#endif 
  json_object* root = json_object_from_file (config_file_name);
  if (NULL == root) {
    fprintf (stderr, "could not open config file %s\n", config_file_name);
    exit (-1);
  }
  
  add_outputs_from_json (root);
  add_sensors_from_json (root);
    
}

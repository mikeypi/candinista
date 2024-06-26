
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
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

sensor_descriptor* sensor_descriptors;
output_descriptor* output_descriptors;
frame_descriptor* frame_descriptors;
top_level_descriptor* top_level_descriptors;

int sensor_count;
int output_count;
int frame_count;
int top_level_count;


static json_object*
get_object_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if ((NULL == x) && (0 != warn)) {
    fprintf (stderr, "required field %s not found\n", field);
    return (0);
  }
 
  return (x);
}


static const char*
get_string_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if ((NULL == x) && (0 != warn)) {
    fprintf (stderr, "required field %s not found\n", field);
    return (0);
  }
  
  return (json_object_get_string (x));
}


static int
get_int_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if ((NULL == x) && (0 != warn)) {
    return (-1);
  }
  
  return (json_object_get_int (x));
}


static float
get_float_from_json (json_object* root, char* field, int warn) {
  json_object* x = json_object_object_get (root, field);

  if ((NULL == x) && (0 != warn)) {
    return (-1.0);
  }
  
  return ((float) json_object_get_double (x));
}


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


static frame_descriptor*
frame_descriptor_by_name (const char* name) {
  int i;
  for (i = 0; i < frame_count; i++) {
    if (0 == strcmp (frame_descriptors[i].name, name)) {
      return (&frame_descriptors[i]);
    }
  }

  fprintf (stderr, "required frame descriptor with name %s not found\n", name);
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
add_sensors_from_json (json_object* root) {
  json_object* sensors = get_object_from_json (root, "sensors", 1);  

  sensor_count = json_object_array_length (sensors);
  sensor_descriptors = (sensor_descriptor*) calloc (sensor_count * sizeof (sensor_descriptor), sizeof (char));
  
  for (int i = 0; i < sensor_count; i++) {
    json_object* e = json_object_array_get_idx (sensors, i);

    const char* name_str = get_string_from_json (e, "name", 1);
    sensor_descriptors[i].name = (char*) calloc (1 + strlen (name_str), sizeof (char));
    strcpy (sensor_descriptors[i].name, name_str);

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
  
    sensor_descriptors[i].offset = get_float_from_json (e, "offset", 1);

#ifdef DEBUG
    printf ("added sensor %s with %d interpolation points\n", name_str, y_value_count);
#endif
  }

#ifdef DEBUG
  printf ("added %d sensors\n", sensor_count);
#endif
  return (sensor_count);
}


static int
add_outputs_from_json (json_object* root) {
  json_object* outputs = get_object_from_json (root, "outputs", 1);
  output_count = json_object_array_length (outputs);
  output_descriptors = (output_descriptor*) calloc (output_count * sizeof (output_descriptor), sizeof (char));
  
  for (int i = 0; i < output_count; i++) {
    json_object* e = json_object_array_get_idx (outputs, i);

    const char *name_str = get_string_from_json (e, "name", 1);
    output_descriptors[i].name = (char*) calloc (1 + strlen (name_str), sizeof (char));
    strcpy (output_descriptors[i].name, name_str);

    const char *label_str = get_string_from_json (e, "label", 1);
    output_descriptors[i].label = (char*) calloc (1 + strlen (label_str), sizeof (char));
    strcpy (output_descriptors[i].label, label_str);

    const char *format_str = get_string_from_json (e, "output format", 1);
    output_descriptors[i].output_format = (char*) calloc (1 + strlen (format_str), sizeof (char));
    strcpy (output_descriptors[i].output_format, format_str);

    output_descriptors[i].box_number = get_int_from_json (e, "box number", 1);
    output_descriptors[i].min = get_float_from_json (e, "minimum value", 0);
    output_descriptors[i].max = get_float_from_json (e, "maximum value", 0);

    output_descriptors[i].update_interval = get_int_from_json (e, "update interval", 0);
    output_descriptors[i].update_floor = get_float_from_json (e, "update floor", 0);

    const char *unit_str = get_string_from_json (e, "units", 1);
    output_descriptors[i].units = enum_from_unit_str (unit_str);
  }

#ifdef DEBUG
  printf ("added %d outputs\n", output_count);
#endif
  return (output_count);
}


static int
add_frames_from_json (json_object* root) {
  json_object* frames = get_object_from_json (root, "frames", 1);  

  frame_count = json_object_array_length (frames);
  frame_descriptors = (frame_descriptor*) calloc (frame_count * sizeof (frame_descriptor), sizeof (char));
  
  for (int i = 0; i < frame_count; i++) {
    json_object* e = json_object_array_get_idx (frames, i);

    const char* name_str = get_string_from_json (e, "name", 1);
    frame_descriptors[i].name = (char*) calloc (1 + strlen (name_str), sizeof (char));
    strcpy (frame_descriptors[i].name, name_str);

    /* json doesn't support hex and hex is the normal way to think of ids. So the id is read as a string (and
     * must be quoted in the .jso)n and the converted to a long.
     */
    const char* id_str = get_string_from_json (e, "id", 1);
    frame_descriptors[i].id = strtol (id_str, NULL, 0);
    
    frame_descriptors[i].field_count = get_int_from_json (e, "field count", 1);
    
    json_object* field_offsets = get_object_from_json (e, "field offsets", 1);
    for (int j = 0; j < json_object_array_length (field_offsets); j++) {
      frame_descriptors[i].field_offsets[j] = json_object_get_int (json_object_array_get_idx (field_offsets, j));
    }

    json_object* field_sizes = get_object_from_json (e, "field sizes", 1);
    for (int j = 0; j < json_object_array_length (field_sizes); j++) {
      frame_descriptors[i].field_sizes[j] = json_object_get_int (json_object_array_get_idx (field_sizes, j));
    }

#ifdef DEBUG
    printf ("added frame for %s\n", name_str);
#endif
  }

#ifdef DEBUG
  printf ("added %d frames\n", frame_count);
#endif
  return (frame_count);
}

  
static int
add_top_levels_from_json (json_object* root) {
  json_object* top_levels = get_object_from_json (root, "top level descriptors", 1);  

  top_level_count = json_object_array_length (top_levels);
  top_level_descriptors = (top_level_descriptor*) calloc (top_level_count * sizeof (top_level_descriptor), sizeof (char));
  
  for (int i = 0; i < top_level_count; i++) {
    json_object* e = json_object_array_get_idx (top_levels, i);

    const char* name_str = get_string_from_json (e, "frame", 1);
    top_level_descriptors[i].frame_descriptor = frame_descriptor_by_name (name_str);

    json_object* sensor_list = get_object_from_json (e, "sensor inputs", 1);
    top_level_descriptors[i].sensor_descriptor_count =  json_object_array_length (sensor_list);
    for (int j = 0; j < top_level_descriptors[i].sensor_descriptor_count; j++) {
      top_level_descriptors[i].sensor_descriptors[j]
	=  sensor_descriptor_by_name (json_object_get_string (json_object_array_get_idx (sensor_list, j)));
    }

    sensor_list = get_object_from_json (e, "sensor outputs", 1);
    top_level_descriptors[i].output_descriptor_count =  json_object_array_length (sensor_list);
    for (int j = 0; j < top_level_descriptors[i].output_descriptor_count; j++) {
      top_level_descriptors[i].output_descriptors[j]
	=  output_descriptor_by_name (json_object_get_string (json_object_array_get_idx (sensor_list, j)));
    }
  }

#ifdef DEBUG
  printf ("added %d top_levels\n", top_level_count);
#endif
  return (top_level_count);
}


void
read_config_from_json (void)
{
  json_object* root = json_object_from_file (config_file_name);
  if (NULL == root) {
    fprintf (stderr, "could not open config file %s\n", config_file_name);
    exit (-1);
  }
  
  add_sensors_from_json (root);
  add_outputs_from_json (root);
  add_frames_from_json (root);
  add_top_levels_from_json (root);
}



#ifndef YAMLLOADER_H
#define YAMLLOADER_H

#include "sensor.h"
#include "panel.h"

typedef struct {
  Sensor** sensors;
  int sensor_count;
  d2_vp_array* sensor_array;
  
  d2_int_array* active_z_index;

  Panel** panels;
  int panel_count;
  d3_array* panel_array;
  
  int x_dimension;
  int y_dimension;

  int panel_z_dimension;
  int sensor_z_dimension;
} Configuration;

Configuration* configuration_load_yaml (const char *path);
void build_tables (Configuration* cfg);
void configuration_free (Configuration*);

Panel* cfg_get_panel (Configuration* cfg, int x_index, int y_index, int z_index);
void cfg_set_panel (Configuration* cfg, Panel* p, int x_index, int y_index, int z_index);

Sensor* cfg_get_sensor (Configuration* cfg, int x_index, int y_index, int z);
void cfg_set_sensor (Configuration* cfg, Sensor* s, int x_index, int y_index, int z_index);

int get_active_z (Configuration* cfg, int x_index, int y_index);
void set_active_z (Configuration* cfg, int x_index, int y_index, int value);
#endif

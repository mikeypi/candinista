#ifndef YAMLLOADER_H
#define YAMLLOADER_H

#include "sensor.h"
#include "panel.h"

typedef struct {
  Sensor **sensors;
  size_t sensor_count;
  
  Panel  **panels;
  size_t panel_count;

  int *active_z_index;
  
  unsigned int x_dimension;
  unsigned int y_dimension;
  unsigned int z_dimension;
} Configuration;

Configuration configuration_load_yaml (const char *path);
Configuration build_tables (Configuration cfg);
void      configuration_free (Configuration *);

Panel* cfg_get_panel (Configuration* cfg, unsigned int x_index, unsigned int y_index, unsigned int z_index);
void cfg_set_panel (Configuration* cfg, Panel* p, unsigned int x_index, unsigned int y_index, unsigned int z_index);

Sensor* cfg_get_sensor (Configuration* cfg, unsigned int x_index, unsigned int y_index);
void cfg_set_sensor (Configuration* cfg, Sensor* s, unsigned int x_index, unsigned int y_index);

int get_active_z (Configuration* cfg, unsigned int x_index, unsigned int y_index);
void set_active_z (Configuration* cfg, unsigned int x_index, unsigned int y_index, unsigned int value);


#endif

#ifndef YAMLLOADER_H
#define YAMLLOADER_H

#include "sensor.h"
#include "panel.h"

typedef struct {
  Sensor **sensors;
  size_t sensor_count;
  
  Panel  **panels;
  size_t panel_count;

  unsigned int n_rows;
  unsigned int n_columns;
} Configuration;

Configuration configuration_load_yaml (const char *path);
Configuration build_tables (Configuration cfg);
void      configuration_free (Configuration *);

Panel* get_panel (Configuration* cfg, unsigned int row, unsigned int column);
Sensor* get_sensor (Configuration* cfg, unsigned int row, unsigned int column);
#endif

#ifndef YAMLLOADER_H
#define YAMLLOADER_H

#include "sensor.h"
#include "panel.h"

typedef struct {
  Sensor **sensors;
  size_t sensor_count;
  Panel  **panels;
  size_t panel_count;
} Configuration;

Configuration configuration_load_yaml (const char *path);
void      configuration_free (Configuration *);

#endif

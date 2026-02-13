#ifndef YAMLLOADER_H
#define YAMLLOADER_H

typedef struct {
  Sensor** sensors;
  size_t sensor_count;

  Panel** panels;
  size_t panel_count;
  
  d3_array* sensor_array;
  d3_array* panel_array;
  
  int x_dimension;
  int y_dimension;
  int z_dimension;

  d3_array* active_z_index;
} Configuration;

Configuration* configuration_load_yaml (const char *path);

void cfg_build_tables (Configuration* cfg);
void cfg_free (Configuration*);

static inline Panel* cfg_get_panel (Configuration* cfg, int i, int j, int k) {
  return (get_item_in_d3_array (cfg -> panel_array, i, j, k));
}

Sensor* cfg_get_sensor (Configuration* cfg, int x_index, int y_index, int z);
void cfg_set_sensor (Configuration* cfg, Sensor* s, int x_index, int y_index, int z_index);

int cfg_get_active_z (Configuration* cfg, int x_index, int y_index);
void cfg_set_active_z (Configuration* cfg, int x_index, int y_index, int value);

extern Configuration* cfg;
#endif

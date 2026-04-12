#ifndef YAMLLOADER_H
#define YAMLLOADER_H

typedef struct {
    Panel* first;
    Panel* last;
    Panel* current;
} panel_group;


typedef struct {
    int can_id;
    Sensor* first;
    Sensor* last;
    panel_group* linked_panel_group;
} sensor_group;

typedef struct {
    Sensor** sensors;
    int sensor_count;

    Panel** panels;
    int panel_count;

    panel_group* panel_groups;
    int panel_group_count;
    sensor_group* sensor_groups;
    int sensor_group_count;
    
    d3_array* panel_array;
    d3_array* active_layer_index;
  
    int x_dimension;
    int y_dimension;
    int z_dimension;

//    can_group* can_groups;
} Configuration;

Configuration* configuration_load_yaml (const char *path);

void cfg_build_tables (Configuration* cfg);
void cfg_free (Configuration*);

static inline Panel* cfg_get_panel (Configuration* cfg, int i, int j, int k) {
    return ((Panel*)get_item_in_d3_array (cfg -> panel_array, i, j, k));
}

Sensor* cfg_get_sensor (Configuration* cfg, int column_index, int row_index, int z);
void cfg_set_sensor (Configuration* cfg, Sensor* s, int column_index, int row_index, int layer_index);

long cfg_get_active_z (Configuration* cfg, int column_index, int row_index);
void cfg_set_active_z (Configuration* cfg, int column_index, int row_index, long value);

extern Configuration* cfg;
#endif

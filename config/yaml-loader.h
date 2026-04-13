#ifndef YAMLLOADER_H
#define YAMLLOADER_H

typedef struct {
    Panel** first;
    Panel** last;
    Panel** current;
} panel_group;


typedef struct {
    int can_id;
    Sensor* first;
    Sensor* last;
    panel_group* linked_panel_group;
} sensor_group;

typedef struct {
    Sensor* sensors;
    int sensor_count;

    Panel** panels;
    int panel_count;

    panel_group* panel_groups;
    int panel_group_count;
  
    sensor_group* sensor_groups;
    int sensor_group_count;
} Configuration;

Configuration* configuration_load_yaml (const char *path);

void cfg_build_tables (Configuration* cfg);
void cfg_free (Configuration*);

extern Configuration* cfg;
#endif

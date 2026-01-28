#ifndef SENSOR_H
#define SENSOR_H

/* Only used to create sensors */
typedef struct {
  char name[64];
  int can_id;
  int can_data_offset;
  int can_data_width;
  double* x_values;
  double* y_values;
  size_t n_values;
  double scale;
  double offset;
  int x_index;
  int y_index;
  int id;
} SensorParameters;

typedef struct Sensor Sensor;

Sensor* sensor_create (SensorParameters* st);
void    sensor_destroy (Sensor*);
void sensor_print (const Sensor* s);

const char* sensor_get_name (const Sensor*);
int  sensor_get_can_id (const Sensor*);
int  sensor_get_can_data_offset (const Sensor*);
int  sensor_get_can_data_width (const Sensor*);
double* sensor_get_x_values (const Sensor*);
double* sensor_get_y_values (const Sensor*);
int sensor_get_n_values (const Sensor*);
int sensor_get_x_index (const Sensor* s);
int sensor_get_y_index (const Sensor* s);
int sensor_get_z_index (const Sensor* s);
double sensor_get_offset (const Sensor*);
double sensor_get_scale (const Sensor*);
int sensor_get_id (const Sensor*);

#endif

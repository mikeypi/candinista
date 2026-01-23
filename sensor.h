#ifndef SENSOR_H
#define SENSOR_H

typedef struct Sensor Sensor;

Sensor* sensor_create (int x_dimension, int y_dimension,
		       const char *name, int can_id, int offset, int width);
void    sensor_destroy (Sensor*);

const char* sensor_get_name (const Sensor*);
double  sensor_get_value (const Sensor*);

int  sensor_get_can_id (const Sensor*);
int  sensor_get_can_data_offset (const Sensor*);
int  sensor_get_can_data_width (const Sensor*);

double* sensor_get_x_values (const Sensor*);
double* sensor_get_y_values (const Sensor*);
int sensor_get_n_values (const Sensor*);

int sensor_get_x_index (const Sensor* s);
int sensor_get_y_index (const Sensor* s);
int sensor_get_z_index (const Sensor* s);

int sensor_get_id (const Sensor* s);

void sensor_set_id (Sensor* s, int id);
void sensor_set_x_values (Sensor* s, double* v, int n);
void sensor_set_y_values (Sensor* s, double* v, int n);

#endif

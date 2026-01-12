#ifndef SENSOR_H
#define SENSOR_H

typedef struct Sensor Sensor;

Sensor* sensor_create (unsigned int x_dimension, unsigned int y_dimension,
		       const char *name, int can_id, int offset, int width);
void    sensor_destroy (Sensor*);

const char* sensor_name (const Sensor*);
double  sensor_value (const Sensor*);
void    sensor_set_value (Sensor*, double);

double  sensor_min (const Sensor*);
double  sensor_max (const Sensor*);

int  sensor_can_id (const Sensor*);
int  sensor_can_data_offset (const Sensor*);
int  sensor_can_data_width (const Sensor*);

double* sensor_x_values (const Sensor*);
double* sensor_y_values (const Sensor*);
int sensor_number_of_interpolation_points (const Sensor*);

void sensor_set_x_values (Sensor* s, double* v, int n);
void sensor_set_y_values (Sensor* s, double* v, int n);

int sensor_y_index (const Sensor* s);
int sensor_x_index (const Sensor* s);

unsigned int sensor_id (const Sensor* s);
void sensor_set_id (Sensor* s, unsigned int id);
#endif

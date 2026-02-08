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

struct Sensor {
  char  *name;
  int can_id;
  int can_data_offset;
  int can_data_width;
  double* x_values;
  double* y_values;
  int n_values;
  int x_index;
  int y_index;
  int z_index;
  double offset;
  double scale;
  int id;
};

typedef struct Sensor Sensor;

static inline const char *sensor_get_name (const Sensor *s)                         { return s -> name; }
static inline int sensor_get_can_id (const Sensor *s)                               { return s -> can_id; }
static inline int sensor_get_can_data_offset (const Sensor *s)                      { return s -> can_data_offset; }
static inline int sensor_get_can_data_width (const Sensor *s)                       { return s -> can_data_width; }
static inline double* sensor_get_x_values (const Sensor *s)                         { return s -> x_values; }
static inline double* sensor_get_y_values (const Sensor *s)                         { return s -> y_values; }
static inline int sensor_get_n_values (const Sensor *s)                             { return s -> n_values; };

static inline int sensor_get_x_index (const Sensor *s)                              { return s -> x_index; }
static inline int sensor_get_y_index (const Sensor *s)                              { return s -> y_index; }
static inline int sensor_get_z_index (const Sensor *s)                              { return s -> z_index; }

static inline double sensor_get_offset (const Sensor *s)                            { return s -> offset; }
static inline double sensor_get_scale (const Sensor *s)                             { return s -> scale; }
static inline int sensor_get_id (const Sensor *s)                                   { return s -> id; }

Sensor* sensor_create (SensorParameters* st);
void    sensor_destroy (Sensor*);
void    sensor_print (FILE* fp, const Sensor* s);
#endif

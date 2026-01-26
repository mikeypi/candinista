#ifndef PANEL_H
#define PANEL_H

typedef enum panel_type {RADIAL_PRESSURE_PANEL, RADIAL_TEMPERATURE_PANEL, LINEAR_PRESSURE_PANEL, LINEAR_TEMPERATURE_PANEL, INFO_PANEL, TPMS_PANEL, GPS_PANEL, UNKNOWN_PANEL} panel_type;

typedef struct Panel Panel;

struct PanelVTable {
  void (*draw)(const struct Panel*, void*);
  double (*get_min) (const struct Panel*);
  double (*get_max) (const struct Panel*);
  double (*get_high_warn) (const Panel*);
  double (*get_low_warn) (const Panel*);
  unit_type (*get_units) (const Panel*);
  unit_type (*get_pressure_units) (const Panel*);
  unit_type (*get_temperature_units) (const Panel*);
  char* (*get_label) (const Panel*);
  char* (*get_output_format) (const Panel*);

  void (*set_minmax) (Panel* g, double min, double max);
  void (*set_warn) (Panel* g, double low, double high);
  void (*set_units) (Panel* g, unit_type ut);
  void (*set_label) (Panel* g, char* label);
  void (*set_value) (Panel* g, double value, int sensor_count);
  void (*set_output_format) (Panel* g, char* value);
};

struct Panel {
  const struct PanelVTable *vtable;
  void (*draw)(void* area, cairo_t* cr, int height, int width, void* p);
  double (*get_min) (const Panel* g);
  double (*get_max) (const Panel* g);
  double (*get_high_warn) (const Panel*);
  double (*get_low_warn) (const Panel*);
  unit_type (*get_units) (const Panel* g);
  unit_type (*get_pressure_units) (const Panel* g);
  unit_type (*get_temperature_units) (const Panel* g);
  char* (*get_label) (const Panel*);
  char* (*get_output_format) (const Panel*);

  void (*set_minmax) (Panel* g, double min, double max);
  void (*set_warn) (Panel* g, double low, double high);
  void (*set_units) (Panel* g, unit_type ut);
  void (*set_label) (Panel* g, char* label);
  void (*set_value) (Panel* g, double value, int sensor_count);
  void (*set_output_format) (Panel* g, char* value);
  int x_index;
  int y_index;
  int z_index;
  unsigned char border;
  int timeout;
  int id;
  int foreground_color;
  int background_color;
  int high_warn_color;
  int low_warn_color;
  panel_type type;
};

/* lifecycle */
Panel* create_linear_gauge_panel (int x_index, int y_index, int z_index, double max, double min);
Panel* create_radial_gauge_panel (int x_index, int y_index, int z_index, double max, double min);
Panel* create_info_panel (int x_index, int y_index, int z_index);
Panel* create_tpms_panel (int x_index, int y_index, int z_index);
Panel* create_gps_panel (int x_index, int y_index, int z_index);
void   panel_destroy (Panel* g);

/* state */
void   panel_set_value (Panel* g, double value, int sensor_count);
void   panel_set_warn (Panel* g, double low, double high);
void   panel_set_minmax (Panel* g, double low, double high);
void   panel_set_label (Panel* g, char* label);
void   panel_set_legend (Panel* g, char* legend);
void   panel_set_border (Panel* g, unsigned char on);
void   panel_set_units(Panel* g, unit_type);
void   panel_set_panel_id(Panel* g, int id);
void   panel_set_timeout(Panel* g, int tm);
void   panel_set_id(Panel* g, int id);
void   panel_set_foreground_color(Panel* g, int color);
void   panel_set_background_color(Panel* g, int color);
void   panel_set_low_warn_color(Panel* g, int color);
void   panel_set_high_warn_color(Panel* g, int color);
void   panel_set_output_format (Panel* g, char* format);
void   panel_set_type (Panel* g, panel_type type);

double panel_get_min (const Panel* g);
double panel_get_max (const Panel* g);
double panel_get_low_warn (const Panel* g);
double panel_get_high_warn (const Panel* g);
int panel_get_y_index (const Panel* g);
int panel_get_x_index (const Panel* g);
int panel_get_z_index (const Panel* g);
int panel_get_panel_id (const Panel* g);
int panel_get_timeout (const Panel* g);
int panel_get_id (const Panel* g);
int panel_get_foreground_color(const Panel* g);
int panel_get_background_color(const Panel* g);
int panel_get_low_warn_color(const Panel* g);
int panel_get_high_warn_color(const Panel* g);
panel_type panel_get_type (const Panel* g);
unit_type panel_get_units (const Panel* g);
unit_type panel_get_pressure_units (const Panel* g);
unit_type panel_get_temperature_units (const Panel* g);

char* panel_get_label (const Panel* g);
char* panel_get_output_format (const Panel* g);

/* rendering */
void   panel_draw (Panel* g, void* cairo_ctx);

double convert_units (double temp, unit_type to);
int get_active_foreground_color (Panel *g, double value, double high_warn, double low_warn);
#endif

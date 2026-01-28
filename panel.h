#ifndef PANEL_H
#define PANEL_H

typedef enum panel_type {RADIAL_PRESSURE_PANEL, RADIAL_TEMPERATURE_PANEL, LINEAR_PRESSURE_PANEL,
			 LINEAR_TEMPERATURE_PANEL, INFO_PANEL, TPMS_PANEL, GPS_PANEL, UNKNOWN_PANEL} panel_type;

typedef struct Panel Panel;

/* Only used to create panels */
typedef struct {
  double min;
  double max;
  double low_warn;
  double high_warn;
  unit_type units;
  unit_type pressure_units;
  unit_type temperature_units; 
  char label[80];
  int border;
  int x_index;
  int y_index;
  int z_index;
  int timeout;
  int id;
  int foreground_color;
  int background_color;
  int high_warn_color;
  int low_warn_color;
  char output_format[80];
  panel_type type;
} PanelParameters;

struct PanelVTable {
  void (*draw)(const struct Panel*, void*);
  void (*set_minmax) (Panel* g, double min, double max);
  void (*set_warn) (Panel* g, double low, double high);
  void (*set_units) (Panel* g, unit_type ut);
  void (*set_label) (Panel* g, char* label);
  void (*set_value) (Panel* g, double value, int sensor_count, int can_id);
  void (*set_output_format) (Panel* g, char* value);
  void (*print) (FILE* fp, const Panel* g);
};

struct Panel {
  const struct PanelVTable *vtable;
  void (*draw)(void* area, cairo_t* cr, int height, int width, void* p);
  void (*print) (FILE* fp, const Panel* g);
  void (*set_minmax) (Panel* g, double min, double max);
  void (*set_warn) (Panel* g, double low, double high);
  void (*set_units) (Panel* g, unit_type ut);
  void (*set_label) (Panel* g, char* label);
  void (*set_value) (Panel* g, double value, int sensor_count, int can_id);
  void (*set_output_format) (Panel* g, char* value);
  panel_type type;
  int x_index;
  int y_index;
  int z_index;
  unsigned char border;
  int foreground_color;
  int background_color;
  int high_warn_color;
  int low_warn_color;
  int timeout;
  int id;
};

/* lifecycle */
Panel* create_linear_gauge_panel (PanelParameters* p);
Panel* create_radial_gauge_panel (PanelParameters* p);
Panel* create_info_panel (PanelParameters* p);
Panel* create_tpms_panel (PanelParameters* p);
Panel* create_gps_panel (PanelParameters* p);
Panel* panel_init_base (PanelParameters* p, Panel* lg);
void   panel_destroy (Panel* g);

/* state */
void   panel_set_value (Panel* g, double value, int sensor_count, int can_id);
void   panel_print (FILE* fp, const Panel* g);

int panel_get_y_index (const Panel* g);
int panel_get_x_index (const Panel* g);
int panel_get_z_index (const Panel* g);
int panel_get_panel_id (const Panel* g);
int panel_get_timeout (const Panel* g);
int panel_get_id (const Panel* g);
panel_type panel_get_type (const Panel* g);

char* panel_get_label (const Panel* g);
char* panel_get_output_format (const Panel* g);

/* rendering */
void   panel_draw (Panel* g, void* cairo_ctx);

double convert_units (double temp, unit_type to);
int get_active_foreground_color (Panel *g, double value, double high_warn, double low_warn);
#endif

#ifndef PANEL_H
#define PANEL_H

typedef enum panel_type {RADIAL_PANEL, LINEAR_PANEL, INFO_PANEL, UNKNOWN_PANEL} panel_type;

typedef struct Panel Panel;

struct PanelVTable {
  void (*draw)(const struct Panel*, void*);
  double (*get_min) (const struct Panel*);
  double (*get_max) (const struct Panel*);
  double (*get_high_warn) (const Panel*);
  double (*get_low_warn) (const Panel*);
  double (*get_offset) (const Panel*);
  unit_type (*get_units) (const Panel*);
  char* (*get_label) (const Panel*);
  double (*get_value) (const Panel*);

  void (*set_minmax) (Panel* g, double min, double max);
  void (*set_warn) (Panel* g, double low, double high);
  void (*set_offset) (Panel* g, double value);
  void (*set_units) (Panel* g, unit_type ut);
  void (*set_label) (Panel* g, char* label);
  void (*set_value) (Panel* g, double value);
  void (*set_output_format) (Panel* g, char* value);
};

struct Panel {
  const struct PanelVTable *vtable;
  void (*draw)(void* area, cairo_t* cr, int height, int width, void* p);
  double (*get_min) (const Panel* g);
  double (*get_max) (const Panel* g);
  unit_type (*get_units) (const Panel* g);
  void (*set_offset) (Panel* g, double value);
  void (*set_warn) (Panel* g, double low, double high);
  void (*set_minmax) (Panel* g, double min, double max);
  void (*set_label) (Panel* g, char* label);
  void (*set_units) (Panel* g, unit_type ut);
  unsigned int x_index;
  unsigned int y_index;
  unsigned int z_index;
  unsigned char border;
  unsigned int timeout;
  unsigned int id;
  unsigned int foreground_color;
  unsigned int background_color;
  unsigned int high_warn_color;
  unsigned int low_warn_color;
  panel_type type;
};

/* lifecycle */
Panel* create_linear_gauge_panel (unsigned int x_index, unsigned int y_index, unsigned int z_index, double max, double min);
Panel* create_radial_gauge_panel (unsigned int x_index, unsigned int y_index, unsigned int z_index, double max, double min);
Panel* create_info_panel (unsigned int x_index, unsigned int y_index, unsigned int z_index);
void   panel_destroy (Panel* g);

/* state */
void   panel_set_value (Panel* g, double value);
void   panel_set_offset (Panel *g, double value);
void   panel_set_warn (Panel* g, double low, double high);
void   panel_set_minmax (Panel* g, double low, double high);
void   panel_set_label (Panel* g, char* label);
void   panel_set_legend (Panel* g, char* legend);
void   panel_set_border (Panel* g, unsigned char on);
void   panel_set_units(Panel* g, unit_type);
void   panel_set_panel_id(Panel* g, unsigned int id);
void   panel_set_timeout(Panel* g, unsigned int tm);
void   panel_set_id(Panel* g, unsigned int id);
void   panel_set_foreground_color(Panel* g, unsigned int color);
void   panel_set_background_color(Panel* g, unsigned int color);
void   panel_set_low_warn_color(Panel* g, unsigned int color);
void   panel_set_high_warn_color(Panel* g, unsigned int color);
void   panel_set_output_format (Panel* g, char* format);
void   panel_set_type (Panel* g, panel_type type);

double panel_get_min (const Panel* g);
double panel_get_max (const Panel* g);
double panel_get_low_warn (const Panel* g);
double panel_get_high_warn (const Panel* g);
unsigned int panel_get_y_index (const Panel* g);
unsigned int panel_get_x_index (const Panel* g);
unsigned int panel_get_z_index (const Panel* g);
unsigned int panel_get_panel_id (const Panel* g);
unsigned int panel_get_timeout (const Panel* g);
unsigned int panel_get_id (const Panel* g);
unsigned int panel_get_foreground_color(Panel* g);
unsigned int panel_get_background_color(Panel* g);
unsigned int panel_get_low_warn_color(Panel* g);
unsigned int panel_get_high_warn_color(Panel* g);
panel_type panel_get_type (Panel* g);
unit_type panel_get_units (Panel* g);

char* panel_get_label (const Panel* g);
char* panel_get_legend (const Panel* g);

/* rendering */
void   panel_draw (Panel* g, void* cairo_ctx);

double convert_units (double temp, unit_type to);
unsigned int get_active_foreground_color (Panel *g, double value, double high_warn, double low_warn);
#endif

#ifndef PANEL_H
#define PANEL_H

typedef struct Panel Panel;

struct PanelVTable {
  void (*draw)(const struct Panel*, void* cr);
};

struct Panel {
  const struct PanelVTable *vtable;
  void (*draw)(void* area, cairo_t* cr, int height, int width, void* p);
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;
  double offset;
  unsigned int x_index;
  unsigned int y_index;
  unsigned int z_index;
  char label[64];
  char legend[64];
  unsigned char border;
  unit_type units;
  unsigned int panel_id;
  unsigned int timeout;
  unsigned int layer;
  unsigned int id;
};

/* lifecycle */
Panel* create_linear_gauge_panel (unsigned int x_index, unsigned int y_index, unsigned int z_index, double max, double min);
Panel* create_radial_gauge_panel (unsigned int x_index, unsigned int y_index, unsigned int z_index, double max, double min);
Panel* create_info_panel (unsigned int x_index, unsigned int y_index, unsigned int z_index);
void   panel_destroy (Panel* g);

/* state */
void   panel_set_value (Panel *g, double value);
void   panel_set_offset (Panel *g, double value);
void   panel_set_warn (Panel *g, double low, double high);
void   panel_set_minmax (Panel *g, double low, double high);
void   panel_set_label (Panel *g, char* label);
void   panel_set_legend (Panel *g, char* legend);
void   panel_set_border (Panel *g, unsigned char on);
void   panel_set_units(Panel *, unit_type);
void   panel_set_panel_id(Panel *, unsigned int id);
void   panel_set_timeout(Panel *, unsigned int tm);
void   panel_set_id(Panel *, unsigned int id);

double panel_get_min (Panel *g);
double panel_get_max (Panel *g);
unsigned int panel_get_y_index (Panel *g);
unsigned int panel_get_x_index (Panel *g);
unsigned int panel_get_z_index (Panel*);
unsigned int panel_get_panel_id (Panel* g);
unsigned int panel_get_timeout(Panel*);
unsigned int panel_get_id(Panel*);


/* rendering */
void   panel_draw (Panel* g, void* cairo_ctx);

double convert_units (double temp, unit_type to);
#endif

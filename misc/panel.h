#ifndef PANEL_H
#define PANEL_H

#define XRED_RGB    0xff0808
#define XBLUE_RGB   0xe0e0e0
#define XORANGE_RGB 0xffa600
#define XBLACK_RGB  0x000000
#define XGRAY_RGB   0x333333

#define DEFAULT_HIGH_WARN_RGB XRED_RGB
#define DEFAULT_LOW_WARN_RGB XBLUE_RGB
#define DEFAULT_FOREGROUND_RGB XORANGE_RGB
#define DEFAULT_BACKGROUND_RGB XBLACK_RGB

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
  void (*draw) (void *, cairo_t*, int, int, void*);
  void (*set_value) (Panel* g, double value, int sensor_count, int can_id);
  void (*print) (FILE* fp, const Panel* g);
};

struct Panel {
  const struct PanelVTable *vtable;
  void (*draw)(void* area, cairo_t* cr, int height, int width, void* p);
  void (*print) (FILE* fp, const Panel* g);
  panel_type type;
  int x_index;
  int y_index;
  int z_index;
  unsigned char border;
  double low_warn;
  double high_warn;
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

/* base functions (work for all panel types) */
static inline int panel_get_x_index (const Panel* g)            { return (g -> x_index); }
static inline int panel_get_y_index (const Panel* g)            { return (g -> y_index); }
static inline int panel_get_z_index (const Panel* g)            { return (g -> z_index); }
static inline int panel_get_timeout (const Panel* g)            { return (g -> timeout); }
static inline int panel_get_id (const Panel* g)                 { return (g -> id); }
static inline panel_type panel_get_type (const Panel *g)        { return (g -> type); }
static inline void panel_destroy (Panel* g)                     { free (g); }

int get_fg_color (const Panel* g, const double value);

/* virtual functions */
void   panel_set_value (Panel* g, double value, int sensor_count, int can_id);
void   panel_print (FILE* fp, const Panel* g);
void   panel_draw (Panel* g, void* cairo_ctx);


#endif

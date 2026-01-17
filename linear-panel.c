#define XOFFSET 0
#define YOFFSET 40

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>
#include <assert.h>

#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"


typedef struct
{
  double min;
  double max;
  double start_x;
  double start_y;
  double height;
  double width;
  int illuminated;
} bargraph_segment;


/* concrete type */
typedef struct {
  Panel base;
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;
  double offset;
  unit_type units;
  char label[64];
  char* output_format;  

  double bargraph_origin_x;
  double bargraph_origin_y;
  double bargraph_width;
  double bargraph_height;
  int bargraph_segment_count;
  bargraph_segment* bargraph_segments;
} LinearPanel;


void draw_linear_gauge_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  LinearPanel* rp = user_data;
  char buffer[80];
  int i;

  assert (NULL != rp);

  double value = convert_units (rp -> value, rp -> units) + rp -> offset;
  unsigned int foreground_color = get_active_foreground_color (&rp -> base, value, rp -> high_warn, rp -> low_warn);
  unsigned int background_color = rp -> base.background_color;
  
  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);
  
  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, rp -> base.foreground_color, 0.9);
    rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
    cairo_stroke (cr);
  }

  set_rgba (cr, foreground_color, 0.9);

  for (i = 0; i < rp -> bargraph_segment_count; i++) {
    if (value >  rp -> bargraph_segments[i].max) {

      set_rgba (cr, foreground_color, 0.9);
      cairo_rectangle (cr,
		       rp -> bargraph_segments[i].start_x + 2,
		       rp -> bargraph_segments[i].start_y + 2,
		       rp -> bargraph_segments[i].width - 4,
		       rp -> bargraph_segments[i].height - 4);
      cairo_fill (cr);
    }

    set_rgba (cr, background_color, 1.0);
    cairo_rectangle (cr,
		     rp -> bargraph_segments[i].start_x,
		     rp -> bargraph_segments[i].start_y,
		     rp -> bargraph_segments[i].width,
		     rp -> bargraph_segments[i].height);
    cairo_stroke (cr);
  }

  /*
   * Draw labels and current value
   */
  cairo_surface_t *surface =
    cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);

  cairo_select_font_face (
			  cr,
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  set_rgba (cr, foreground_color, 0.9);

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%s", str_from_unit_enum (panel_get_units (&rp -> base)));
  show_text_left_justified (cr, 238 + XOFFSET + 40, 85 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if (NULL != rp -> label) {
    sprintf (buffer, "%s", rp -> label);
    show_text_unjustified (cr, 70 + XOFFSET, 210 + YOFFSET, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }
    
  // Print value field and burn-in 
  cairo_select_font_face (
			  cr,
			  "DSEG7Classic",
			  CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );


  sprintf (buffer, rp -> output_format, value);

  if (3 < strlen (buffer)) {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_text_right_justified (cr,
			       99 + XOFFSET - 20,
			       160 + YOFFSET - 10, 
			       buffer,
			       4,
			       foreground_color,
			       true,
			       true);
  } else {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_text_right_justified (cr,
			       99 + XOFFSET - 20,
			       160 + YOFFSET - 10, 
			       buffer,
			       3,
			       foreground_color,
			       true,
			       true);
  }
  
  cairo_surface_destroy (surface);
}


static double get_min (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> min); }
static double get_max (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> max); }
static double get_high_warn (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> high_warn); }
static double get_low_warn (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> low_warn); }
static double get_offset (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> offset); }
static unit_type get_units (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> units); }
static char* get_label (const Panel* g) { LinearPanel* rp = (LinearPanel*) g; return (rp -> label); }

static void set_minmax (Panel* g, double min, double max) { LinearPanel* rp = (LinearPanel*) g; rp -> min = min; rp -> max = max; }
static void set_warn (Panel* g, double low, double high) { LinearPanel* rp = (LinearPanel*) g; rp -> low_warn = low; rp -> high_warn = high; }
static void set_offset (Panel* g, double offset)  { LinearPanel* rp = (LinearPanel*) g; rp -> offset = offset; }
static void set_units (Panel* g, unit_type ut)  { LinearPanel* rp = (LinearPanel*) g; rp -> units = ut; }
static void set_label (Panel* g, char* label) { LinearPanel* rp = (LinearPanel*) g; strcpy (rp -> label, label); }
static void set_value (Panel* g, double value) { LinearPanel* rp = (LinearPanel*) g; rp -> value = value; }
static void set_output_format (Panel* g, char* format) { LinearPanel* rp = (LinearPanel*) g; rp -> output_format = strdup (format); }

static const struct PanelVTable linear_vtable = {
  .draw = (void (*)(const struct Panel*, void *))draw_linear_gauge_panel,  
  .get_min = (double (*)(const struct Panel*))get_min,
  .get_max = (double (*)(const struct Panel*))get_max,
  .get_high_warn = (double (*)(const struct Panel*))get_high_warn,
  .get_low_warn = (double (*)(const struct Panel*))get_low_warn,
  .get_offset = (double (*)(const struct Panel*))get_offset,
  .get_units = (unit_type (*)(const struct Panel*))get_units,    
  .get_label = (char* (*)(const struct Panel*))get_label,

  .set_minmax = (void (*) (Panel*, double, double))set_minmax,
  .set_warn = (void (*) (Panel*, double, double)) set_warn,
  .set_offset = (void (*) (Panel*, double)) set_offset,
  .set_units = (void (*) (Panel*, unit_type)) set_units,
  .set_label = (void (*) (Panel*, char*)) set_label,
  .set_value = (void (*) (Panel*, double)) set_value,
  .set_output_format = (void (*) (Panel*, char*)) set_output_format  
};

Panel* create_linear_gauge_panel (
				  unsigned int x_index,
  				  unsigned int y_index,
				  unsigned int z_index,
  				  double max,
  				  double min) {

  LinearPanel *lg = g_new0 (typeof (*lg), 1);
  
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_linear_gauge_panel;
  lg -> base.vtable = &linear_vtable;
  lg -> base.x_index = x_index;
  lg -> base.y_index = y_index;
  lg -> base.z_index = z_index;
  lg -> max = max;
  lg -> min = min;
  lg -> output_format = "%.0f";
  
  lg -> base.background_color = XBLACK_RGB;
  lg -> base.foreground_color = XORANGE_RGB;
  lg -> base.high_warn_color = XRED_RGB;
  lg -> base.low_warn_color = XBLUE_RGB;
  
  lg -> bargraph_origin_x = DEFAULT_BARGRAPH_ORIGIN_X;
  lg -> bargraph_origin_y = DEFAULT_BARGRAPH_ORIGIN_Y;
  lg -> bargraph_width = DEFAULT_BARGRAPH_WIDTH;
  lg -> bargraph_height = DEFAULT_BARGRAPH_HEIGHT;
  lg -> bargraph_segment_count = DEFAULT_BARGRPAH_SEGMENT_COUNT;

  lg -> bargraph_segments = (bargraph_segment*) calloc (lg -> bargraph_segment_count, sizeof (bargraph_segment));

  double delta_x = lg -> bargraph_width  / lg -> bargraph_segment_count;
  double total_x =  lg -> bargraph_origin_x;
  double y;

  for (int i = 0; i < lg -> bargraph_segment_count; i++) {
    lg -> bargraph_segments[i].width = delta_x; 
    lg -> bargraph_segments[i].start_x = total_x;
    lg -> bargraph_segments[i].start_y = lg -> bargraph_origin_y;
    y = sqrt (pow (total_x / lg -> bargraph_width, 2) + 1) - 1;
    lg -> bargraph_segments[i].height = (y + 0.1) * lg -> bargraph_height;
    lg -> bargraph_segments[i].max =
      (max - min) * ((double) i / (double) lg -> bargraph_segment_count) + min;
    total_x += delta_x;

  }

  return (Panel*) lg;
}


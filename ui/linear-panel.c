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
  int high_warn_color;
  int low_warn_color;
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
  (void) area;
  LinearPanel* rp = user_data;
  char buffer[80];
  int i;

  assert (NULL != rp);

  double value = convert_units (rp -> value, rp -> units);
  int foreground_color = get_fg_color ((Panel*)rp, value);
  int background_color = rp -> base.background_color;
  
  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);
  
  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, rp -> base.foreground_color, 0.9);
    rounded_rectangle (cr, 5.0, 5.0, height - 10, width - 10, 5.0);
    cairo_stroke (cr);
  }

  cairo_set_line_width (cr, 2.0);
  set_rgba (cr, foreground_color, 0.9);

  for (i = 0; i < rp -> bargraph_segment_count; i++) {
    if (value >  rp -> bargraph_segments[i].max) {

      set_rgba (cr, foreground_color, 1.0);
      cairo_rectangle (cr,
		       rp -> bargraph_segments[i].start_x + 2,
		       rp -> bargraph_segments[i].start_y + 2,
		       rp -> bargraph_segments[i].width - 4,
		       rp -> bargraph_segments[i].height - 4);
      cairo_fill (cr);
    }

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
  cairo_select_font_face (
			  cr,
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  set_rgba (cr, foreground_color, 0.9);

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%s", str_from_unit_enum (rp -> units));
  show_text_left_justified (cr, 238 + XOFFSET + 40, 85 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if ('\0' != rp -> label[0]) {
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

  cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
  sprintf (buffer, rp -> output_format, value);
    
  if (3 < strlen (buffer)) {
    show_text_right_justified (cr, width / 2.0, 160 + YOFFSET, buffer, 4);
    set_rgba (cr, foreground_color, 0.14);
    show_text_burn_in (cr, width / 2.0, 160 + YOFFSET, buffer, 4);
    set_rgba (cr, foreground_color, 0.9);
    show_text_box (cr, width / 2.0, 160 + YOFFSET, buffer, 4);
  } else {
    show_text_right_justified (cr, width / 2.0, 160 + YOFFSET, buffer, 3);
    set_rgba (cr, foreground_color, 0.14);
    show_text_burn_in (cr, width / 2.0, 160 + YOFFSET, buffer, 3);
    set_rgba (cr, foreground_color, 0.9);
    show_text_box (cr, width / 2.0, 160 + YOFFSET, buffer, 3);
  }
}

void print_linear_panel (FILE* fp, const Panel* g) {
  (void) fp;
  (void) g;
}

static void set_value (Panel* g, double value, int sensor_count, int can_id) {
  (void) sensor_count;
  (void) can_id;

  LinearPanel* rp = (LinearPanel*) g;
  rp -> value = value;
}

static const struct PanelVTable linear_vtable = {
  .draw = (void (*)(void *, cairo_t*, int, int, void*)) draw_linear_gauge_panel,
  .print = (void (*) (FILE*, const Panel*)) print_linear_panel,
  .set_value = (void (*) (Panel*, double, int, int)) set_value,
};

Panel* create_linear_gauge_panel (PanelParameters* p) {
  LinearPanel *lg = g_new0 (typeof (*lg), 1);
  
  if ((0 == p-> max) && (0 == p -> min)) {
    fprintf (stderr, "error: max and min not specified for linear gauge\n");
  }

  if (p -> min > p -> max) {
    fprintf (stderr, "error: min greater than max for linear gauge\n");
  }

  lg = (LinearPanel*) panel_init_base (p, (Panel*) lg);
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_linear_gauge_panel;
  lg -> base.vtable = &linear_vtable;
  
  lg -> output_format = "%.0f";
  lg -> max = p -> max;
  lg -> min = p -> min;
  lg -> low_warn_color = p -> low_warn_color;
  lg -> high_warn_color = p -> high_warn_color;
  lg -> units = p -> units;
  strcpy (lg -> label, p -> label);
  
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
      (p -> max - p -> min) * ((double) i / (double) lg -> bargraph_segment_count) + p -> min;
    total_x += delta_x;

  }

  return (Panel*) lg;
}

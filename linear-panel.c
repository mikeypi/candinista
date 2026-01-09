#define XOFFSET 0
#define YOFFSET 40

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>

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
  double bargraph_origin_x;
  double bargraph_origin_y;
  double bargraph_width;
  double bargraph_height;
  int bargraph_segment_count;
  bargraph_segment* bargraph_segments;
  /* layout-specific fields could go here */
} LinearPanel;


void draw_linear_gauge_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  LinearPanel* rp = user_data;
  Panel* p = user_data;
  char buffer[80];
  int i;

  if (NULL == rp) {
    return;
  }

  double value = convert_units (p -> value, p -> units) + p -> offset;

 /*
  * Draw background
  */
  set_rgba_for_background (cr);
  cairo_paint (cr);
  
  if (0 != p -> border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba_for_foreground (cr, get_warning_level (value, p -> high_warn, p -> low_warn));
    rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
    cairo_stroke (cr);
  }

  for (i = 0; i < rp -> bargraph_segment_count; i++) {
    if (p -> value >  rp -> bargraph_segments[i].max) {
      cairo_rectangle (cr,
		       rp -> bargraph_segments[i].start_x,
		       rp -> bargraph_segments[i].start_y,
		       rp -> bargraph_segments[i].width,
		       rp -> bargraph_segments[i].height);
      cairo_fill (cr);
    }

    cairo_rectangle (cr,
		     rp -> bargraph_segments[i].start_x,
		     rp -> bargraph_segments[i].start_y,
		     rp -> bargraph_segments[i].width,
		     rp -> bargraph_segments[i].height);
  
    cairo_stroke (cr);
  }

  if (0 != p -> border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba_for_foreground (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));
    rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
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
  set_rgba_for_foreground (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));

  // Print Label
  if (NULL != p -> label) {
    sprintf (buffer, "%s", p -> label);
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

  sprintf (buffer, "%.0f", p -> value);

  if (3 < strlen (buffer)) {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_text_right_justified (cr,
			       99 + XOFFSET,
			       160 + YOFFSET, 
			       buffer,
			       4,
			       get_warning_level (p -> value, p -> high_warn, p -> low_warn),
			       true,
			       true);
  } else {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_text_right_justified (cr,
			       99 + XOFFSET,
			       160 + YOFFSET, 
			       buffer,
			       3,
			       get_warning_level (p -> value, p -> high_warn, p -> low_warn),
			       true,
			       true);
  }
  
  cairo_surface_destroy (surface);
}


static const struct PanelVTable linear_vtable = {
  .draw = (void (*)(const struct Panel *, void *))draw_linear_gauge_panel
};


Panel* create_linear_gauge_panel (unsigned int row,
				  unsigned int column,
				  double max,
				  double min) {

  LinearPanel *lg = calloc (1, sizeof *lg);
  int i;
  double delta_x = lg -> bargraph_width  / lg -> bargraph_segment_count;
  double total_x =  lg -> bargraph_origin_x;
  double y;
  
  lg -> base.draw = draw_linear_gauge_panel;
  lg -> base.vtable = &linear_vtable;
  lg -> base.row = row;
  lg -> base.column = column;
  lg -> base.max = max;
  lg -> base.min = min;
  
  lg -> bargraph_origin_x = DEFAULT_BARGRAPH_ORIGIN_X;
  lg -> bargraph_origin_y = DEFAULT_BARGRAPH_ORIGIN_Y;
  lg -> bargraph_width = DEFAULT_BARGRAPH_WIDTH;
  lg -> bargraph_height = DEFAULT_BARGRAPH_HEIGHT;
  lg -> bargraph_segment_count = DEFAULT_BARGRPAH_SEGMENT_COUNT;

  if (NULL == lg -> bargraph_segments) {
    /* NULL means this cairo gauge has yet to be initialized and does not mean that an error has occured. */
    lg -> bargraph_segments = (bargraph_segment*) calloc (lg -> bargraph_segment_count, sizeof (bargraph_segment));

    for (i = 0; i < lg -> bargraph_segment_count; i++) {
      lg -> bargraph_segments[i].width = delta_x; 
      lg -> bargraph_segments[i].start_x = total_x;
      lg -> bargraph_segments[i].start_y = lg -> bargraph_origin_y;
      y = sqrt (pow (total_x / lg -> bargraph_width, 2) + 1) - 1;
      lg -> bargraph_segments[i].height = (y + 0.1) * lg -> bargraph_height;
      lg -> bargraph_segments[i].max =
      	(max - min) * ((double) i / (double) lg -> bargraph_segment_count) + min;
      total_x += delta_x;
    }
  }
  return (Panel*) lg;
}


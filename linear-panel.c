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
#include "cairo_panel.h"


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


gboolean
gtk_update_linear_gauge_panel_value (gpointer user_data) {
  struct
  {
    GtkDrawingArea *area;
    Panel *p;
  } *ctx = user_data;
  
  gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));

  return G_SOURCE_CONTINUE;
}


void
gtk_draw_linear_gauge_panel_cb (GtkDrawingArea* area,
                  cairo_t*        cr,
                  int             width,
                  int             height,
                  gpointer        user_data)
{
  Panel* p = user_data;

  g_return_if_fail(p != NULL);
  g_return_if_fail(p->draw != NULL);

  p -> draw (area, cr, width, height, p);
}


#if 0
*static void linear_draw (const Panel* g, void* cr) {
  (void) cr; /* unused for example */
  /*
  double range = g -> max - g -> min;
  double norm = (g -> value - g -> min) / range;

  printf ("Linear panel: %.1f%%\n", norm * 100.0);*/
}
#endif

void draw_linear_gauge_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  LinearPanel* rp = user_data;
  Panel* p = user_data;

  if (NULL == rp) {
    return;
  }

  char buffer[80];
  int i;

  /*
    Initialization section.
  */

  double delta_x = rp -> bargraph_width  / rp -> bargraph_segment_count;
  double total_x =  rp -> bargraph_origin_x;
  double y;
  
  if (NULL == rp -> bargraph_segments) {
    /* NULL means this cairo gauge has yet to be initialized and does not mean that an error has occured. */
    rp -> bargraph_segments = (bargraph_segment*) calloc (rp -> bargraph_segment_count, sizeof (bargraph_segment));

    for (i = 0; i < rp -> bargraph_segment_count; i++) {
      rp -> bargraph_segments[i].width = delta_x; 
      rp -> bargraph_segments[i].start_x = total_x;
      rp -> bargraph_segments[i].start_y = rp -> bargraph_origin_y;
      y = sqrt (pow (total_x / rp -> bargraph_width, 2) + 1) - 1;
      rp -> bargraph_segments[i].height = (y + 0.1) * rp -> bargraph_height;
      rp -> bargraph_segments[i].max =
      	(p -> max - p -> min) * ((double) i / (double) rp -> bargraph_segment_count) + p -> min;
      total_x += delta_x;
    }
  }

 /*
  * Draw background
  */
  set_rgba_for_background (cr);
  //  cairo_set_source_rgba (cr, BACKGROUND);

  rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
  cairo_fill (cr);

  cairo_set_line_width (cr, 2);
  cairo_set_source_rgba (cr, FOREGROUND_RGBA);

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
  .draw = draw_linear_gauge_panel
};


Panel* create_linear_gauge_panel (unsigned int row, unsigned int column) {
  LinearPanel *lg = calloc (1, sizeof *lg);
  lg -> base.draw = draw_linear_gauge_panel;
  lg -> base.vtable = &linear_vtable;
  lg -> base.row = row;
  lg -> base.column = column;

  lg -> bargraph_origin_x = DEFAULT_BARGRAPH_ORIGIN_X;
  lg -> bargraph_origin_y = DEFAULT_BARGRAPH_ORIGIN_Y;
  lg -> bargraph_width = DEFAULT_BARGRAPH_WIDTH;
  lg -> bargraph_height = DEFAULT_BARGRAPH_HEIGHT;
  lg -> bargraph_segment_count = DEFAULT_BARGRPAH_SEGMENT_COUNT;
  return (Panel*) lg;
}


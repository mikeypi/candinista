#define XOFFSET 0
#define YOFFSET 40

#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "cairo_panel.h"

#define DEFAULT_BARGRAPH_ORIGIN_X 20
#define DEFAULT_BARGRAPH_ORIGIN_Y 15
#define DEFAULT_BARGRAPH_WIDTH 300
#define DEFAULT_BARGRAPH_HEIGHT 160
#define DEFAULT_BARGRPAH_SEGMENT_COUNT 16

cairo_bargraph_panel* new_cairo_bargraph_panel () {
  cairo_bargraph_panel* g = g_new0 (cairo_bargraph_panel, 1);
  g -> bargraph_origin_x = DEFAULT_BARGRAPH_ORIGIN_X;
  g -> bargraph_origin_y = DEFAULT_BARGRAPH_ORIGIN_Y;
  g -> bargraph_width = DEFAULT_BARGRAPH_WIDTH;
  g -> bargraph_height = DEFAULT_BARGRAPH_HEIGHT;
  g -> bargraph_segment_count = DEFAULT_BARGRPAH_SEGMENT_COUNT;

  return (g);
}

gboolean
update_cairo_bargraph_panel_value (gpointer user_data)
{
  struct
  {
    GtkDrawingArea* area;
    cairo_bargraph_panel* g;
  }* ctx = user_data;
  
  gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));

  return G_SOURCE_CONTINUE;
}
									 
void draw_cairo_bargraph_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  cairo_bargraph_panel* gd = user_data;
  if (NULL == gd) {
    return;
  }

  char buffer[80];
  int i;

  /*
    Initialization section.
  */

  double delta_x = gd -> bargraph_width  / gd -> bargraph_segment_count;
  double total_x =  gd -> bargraph_origin_x;
  double y;
  
  if (NULL == gd -> bargraph_segments) {
    /* NULL means this cairo gauge has yet to be initialized and does not mean that an error has occured. */
    fprintf (stderr, "cairo bargraph initialization\n");
    gd -> bargraph_segments = (bargraph_segment*) calloc (gd -> bargraph_segment_count, sizeof (bargraph_segment));

    for (i = 0; i < gd -> bargraph_segment_count; i++) {
      gd -> bargraph_segments[i].width = delta_x; 
      gd -> bargraph_segments[i].start_x = total_x;
      gd -> bargraph_segments[i].start_y = gd -> bargraph_origin_y;
      y = sqrt (pow (total_x/gd -> bargraph_width, 2) + 1) - 1;
      gd -> bargraph_segments[i].height = (y + 0.1) * gd -> bargraph_height;
      gd -> bargraph_segments[i].max =
      	(gd -> max - gd -> min) * ((double) i / (double) gd -> bargraph_segment_count) + gd -> min;
      total_x += delta_x;

      fprintf (stderr, "added segment at %f,%f with width %f and height %f for max value %f\n",
	       gd -> bargraph_segments[i].start_x,
	       gd -> bargraph_segments[i].start_y,
	       gd -> bargraph_segments[i].width,
	       gd -> bargraph_segments[i].height,
	       gd -> bargraph_segments[i].max);
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

  for (i = 0; i < gd -> bargraph_segment_count; i++) {
    if (gd -> value >  gd -> bargraph_segments[i].max) {
      cairo_rectangle (cr,
		       gd -> bargraph_segments[i].start_x,
		       gd -> bargraph_segments[i].start_y,
		       gd -> bargraph_segments[i].width,
		       gd -> bargraph_segments[i].height);
      cairo_fill (cr);
    }

    cairo_rectangle (cr,
		     gd -> bargraph_segments[i].start_x,
		     gd -> bargraph_segments[i].start_y,
		     gd -> bargraph_segments[i].width,
		     gd -> bargraph_segments[i].height);
  
    cairo_stroke (cr);
  }

  if (0 != gd -> border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba_for_foreground (cr, get_warning_level (gd -> value, gd -> high_warn, gd -> low_warn));
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
  set_rgba_for_foreground (cr, get_warning_level (gd -> value, gd -> high_warn, gd -> low_warn));

  // Print Label
  if (NULL != gd -> label) {
    sprintf (buffer, "%s", gd -> label);
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

  sprintf (buffer, "%.0f", gd -> value);

  if (3 < strlen (buffer)) {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_text_right_justified (cr,
			       99 + XOFFSET,
			       160 + YOFFSET, 
			       buffer,
			       4,
			       get_warning_level (gd -> value, gd -> high_warn, gd -> low_warn),
			       true,
			       true);
  } else {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_text_right_justified (cr,
			       99 + XOFFSET,
			       160 + YOFFSET, 
			       buffer,
			       3,
			       get_warning_level (gd -> value, gd -> high_warn, gd -> low_warn),
			       true,
			       true);
  }
  
  cairo_surface_destroy (surface);
}

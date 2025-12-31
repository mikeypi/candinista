#include <gtk/gtk.h>
#include <math.h>
#include "cairo_gauge.h"


/* Simple gauge widget implemented with GtkDrawingArea + Cairo (GTK4)
 * Compile with:
 * cc gauge.c `pkg-config --cflags --libs gtk4` -lm -o gauge
 */

cairo_gauge* new_cairo_gauge () {
  cairo_gauge *g = g_new0 (cairo_gauge, 1);
  g -> radius = DEFAULT_RADIUS;
  g -> start_angle = DEFAULT_START_ANGLE;
  g -> end_angle = DEFAULT_END_ANGLE;
  g -> segment_count = DEFAULT_SEGMENT_COUNT;
  g -> segment_gap_size = DEFAULT_SEGMENT_GAP_SIZE;

  return (g);
}


gboolean
update_cairo_gauge_value (gpointer user_data)
{
  struct
  {
    GtkDrawingArea *area;
    cairo_gauge *g;
  } *ctx = user_data;
  
  gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));

  return G_SOURCE_CONTINUE;
}


									 
static void
set_burnin (cairo_t* cr, cairo_gauge* gd) {
  ((gd -> value > gd -> high_warn) || ((gd -> value < gd -> low_warn)))
      ? cairo_set_source_rgba (cr, WARN_BURNIN)
      : cairo_set_source_rgba (cr, BURNIN);
}


static void
set_foreground (cairo_t* cr, cairo_gauge* gd) {
  ((gd -> value > gd -> high_warn) || ((gd -> value < gd -> low_warn)))
      ? cairo_set_source_rgba (cr, WARN)
      : cairo_set_source_rgba (cr, FOREGROUND);
}


static void
show_gauge_text(cairo_t* cr, int x, int y, char* buffer) {
  cairo_move_to (cr, x, y);
  cairo_show_text (cr, buffer);
}


static void
show_gauge_text_a (cairo_t* cr, int x, int y, int x_step, int field_width, char* buffer) {
  int sl = strlen (buffer);
  cairo_move_to (cr, x + (field_width - sl) * x_step, y);
  cairo_show_text (cr, buffer);
}


void draw_cairo_gauge (GtkDrawingArea *area,
		       cairo_t *cr,
		       int width,
		       int height,
		       gpointer user_data)
{
  cairo_gauge* gd = user_data;
  
  char buffer[80];
  
  if (NULL == gd) {
    return;
  }

  /*
    Initialization section.
  */
  
  if (NULL == gd -> arc_segments) {
    /* NULL means this cairo gauge has yet to be initialized and does not mean that an error has occured. */
    gd -> arc_segments = (arc_segment*) malloc (gd -> segment_count * sizeof (arc_segment));

    for (int i = 0; i < gd -> segment_count; i++) {
      double size_subarc_angle = (gd -> end_angle - gd -> start_angle) / gd -> segment_count;
      gd -> arc_segments[i].arc_start_angle = gd -> start_angle + i * size_subarc_angle;
      gd -> arc_segments[i].arc_end_angle = gd -> arc_segments[i].arc_start_angle
	+ size_subarc_angle - gd -> segment_gap_size;
    }
  }

  /*
    Draw background, arc and segments
  */
  
  /* background */
  cairo_set_source_rgba (cr,BACKGROUND);
  cairo_paint (cr);

  set_foreground (cr, gd);

  /* gauge arc */
  cairo_set_line_width (cr, 3.0);
  
  cairo_arc (cr, width / 2.0, height / 2.0,
	     gd -> radius + 10,
	     gd -> start_angle,
	     gd -> end_angle);

  cairo_stroke (cr);
  
  /* illuminated segments */
  cairo_set_line_width (cr, 10.0);

  double t = (gd -> value - gd -> min) / (gd -> max - gd -> min);
  t = CLAMP (t, 0.0, 1.0);
  double angle = gd -> start_angle + t * (gd -> end_angle
					  - gd -> start_angle);

  for (int i = 0; i < gd -> segment_count; i++) {
    if (angle < gd -> arc_segments[i].arc_end_angle) {
      set_burnin (cr, gd);
    }
    else {
      set_foreground (cr, gd);
    }

    cairo_arc (cr, width / 2.0, height / 2.0,
	       gd -> radius,
	       gd -> arc_segments[i].arc_start_angle,
	       gd -> arc_segments[i].arc_end_angle);

    cairo_stroke (cr);
  }

  /*
    Draw labels and current value
  */
  cairo_surface_t *surface =
    cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);

  cairo_select_font_face (
			  cr,
			  "dejavu",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  set_foreground (cr, gd);

  // Print Legend
  if (NULL != gd -> legend) {
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
    sprintf (buffer, "%s", gd -> legend);
    show_gauge_text_a (cr, 184, 85, 18, 4, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }

  // Print Maximum Value
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%.0f", gd -> max);
  show_gauge_text_a (cr, 179, 240, 18, 4, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Minimum Value
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%.0f", gd -> min);
  show_gauge_text (cr, 40, 165, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if (NULL != gd -> label) {
    //    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE + 5);
    sprintf (buffer, "%s", gd -> label);
    show_gauge_text (cr, 70, 210, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }
    
  // Print value field and burn-in 
  cairo_select_font_face (
			  cr,
			  "DSEG7Classic",
			  CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL
			  //			  CAIRO_FONT_WEIGHT_BOLD
			  );


  sprintf (buffer, "%.0f", gd -> value);

  if (3 < strlen (buffer)) {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 10);
    set_burnin (cr, gd);
    show_gauge_text_a (cr, 77, 160, 45, 4, "888");
    set_foreground (cr, gd);
    show_gauge_text_a (cr, 77, 160, 45, 4, buffer);
  } else {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    set_burnin (cr, gd);
    show_gauge_text_a (cr, 39, 160, 53, 4, "888");
    set_foreground (cr, gd);
    show_gauge_text_a (cr, 39, 160, 53, 4, buffer);
  }
  
  cairo_surface_destroy (surface);
}

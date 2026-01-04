#define XOFFSET 0
#define YOFFSET 10

#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "cairo_panel.h"

cairo_gauge_panel* new_cairo_gauge_panel () {
  cairo_gauge_panel* g = g_new0 (cairo_gauge_panel, 1);
  g -> radius = DEFAULT_RADIUS;
  g -> start_angle = DEFAULT_START_ANGLE;
  g -> end_angle = DEFAULT_END_ANGLE;
  g -> segment_count = DEFAULT_SEGMENT_COUNT;
  g -> segment_gap_size = DEFAULT_SEGMENT_GAP_SIZE;

  return (g);
}


gboolean
update_cairo_gauge_panel_value (gpointer user_data)
{
  struct
  {
    GtkDrawingArea *area;
    cairo_gauge_panel *g;
  } *ctx = user_data;
  
  gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));

  return G_SOURCE_CONTINUE;
}

									 
void draw_cairo_gauge_panel (GtkDrawingArea *area,
		       cairo_t *cr,
		       int width,
		       int height,
		       gpointer user_data)
{
  cairo_gauge_panel* gd = user_data;
  
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

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

/*
 * Draw background, arc and segments
  */
  cairo_set_source_rgba (cr, BACKGROUND);
  rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
  cairo_fill (cr);

  cairo_set_line_width (cr, 1.0);
  set_foreground (cr, gd -> value, gd -> high_warn, gd -> low_warn);
  rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
  cairo_stroke (cr);  

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
      set_burnin (cr, gd -> value, gd -> high_warn, gd -> low_warn);
    }
    else {
      set_foreground (cr, gd -> value, gd -> high_warn, gd -> low_warn);
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
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  set_foreground (cr, gd -> value, gd -> high_warn, gd -> low_warn);

  if (NULL != gd -> legend) {
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
    sprintf (buffer, "%s", gd -> legend);
    show_gauge_text_aa (cr, 244 + XOFFSET, 85 + YOFFSET, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }

  // Print Maximum Value
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%.0f", gd -> max);
  show_gauge_text_aa (cr, 255 + XOFFSET, 248 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Minimum Value
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%.0f", gd -> min);
  show_gauge_text_aa (cr, 60 + XOFFSET, 165 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if (NULL != gd -> label) {
    sprintf (buffer, "%s", gd -> label);
    show_gauge_text (cr, 70 + XOFFSET, 210 + YOFFSET, buffer);
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
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 10);
    show_gauge_text_with_burnin (cr,
				 gd -> value, gd -> high_warn, gd -> low_warn,
				 99 + XOFFSET, 160 + YOFFSET, 4, buffer);
  } else {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE);
    show_gauge_text_with_burnin (cr,
				 gd -> value, gd -> high_warn, gd -> low_warn,
				 99 + XOFFSET, 160 + YOFFSET, 3, buffer);
  }
  
  cairo_surface_destroy (surface);
}

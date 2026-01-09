#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>

#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"

#define XOFFSET 0
#define YOFFSET 10


typedef struct
{
  double arc_start_angle;
  double arc_end_angle;
  int illuminated;
} arc_segment;


typedef struct {
  Panel base;

  double radius;
  double start_angle;
  double end_angle;

  int segment_count;
  arc_segment* arc_segments;
  double segment_gap_size;
} RadialPanel;


void draw_radial_gauge_panel (GtkDrawingArea* area,
			      cairo_t* cr,
			      int width,
			      int height,
			      gpointer user_data)
{
  RadialPanel* rp = user_data;
  Panel* p = user_data;
  
  char buffer[80];
  
  if (NULL == rp) {
    return;
  }

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

/*
 * Draw background, arc and segments
 */
  set_rgba_for_background (cr);
  rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
  cairo_fill (cr);

  if (0 != p -> border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba_for_foreground (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));
    rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
    cairo_stroke (cr);
  }

  /* gauge arc */
  cairo_set_line_width (cr, 3.0);
  cairo_arc (cr, width / 2.0, height / 2.0,
	     rp -> radius + 10,
	     rp -> start_angle,
	     rp -> end_angle);
  cairo_stroke (cr);
  
  /* illuminated segments */
  cairo_set_line_width (cr, 7.0);

  double t = CLAMP ((p -> value - p -> min) / (p -> max - p -> min), 0.0, 1.0);
  double angle = rp -> start_angle + t * (rp -> end_angle
					  - rp -> start_angle);

  for (int i = 0; i < rp -> segment_count; i++) {
    if (angle < rp -> arc_segments[i].arc_end_angle) {
      set_rgba_for_burn_in (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));
    }
    else {
      set_rgba_for_foreground (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));
    }

    cairo_arc (cr, width / 2.0, height / 2.0,
	       rp -> radius + 2,
	       rp -> arc_segments[i].arc_start_angle,
	       rp -> arc_segments[i].arc_end_angle);

    cairo_stroke (cr);
  }

  #define ID 11
  #define OD 18

  if ((!isnan (p -> low_warn)) && (!isnan (p -> high_warn)) && (!isnan (p -> min)) && (!isnan (p -> max))) {

    t = CLAMP ((p -> low_warn - p -> min) / (p -> max - p -> min), 0.0, 1.0);
    double range_start = rp -> start_angle + t * (rp -> end_angle -  rp-> start_angle);

    t = CLAMP ((p -> high_warn - p -> min) / (p -> max - p -> min), 0.0, 1.0);
    double range_end = rp -> start_angle + t * (rp -> end_angle - rp -> start_angle);

    double x1 = width / 2 + (rp -> radius + ID) * cos (range_end);
    double y1 = height / 2 + (rp -> radius + ID) * sin (range_end);
    
    set_rgba_for_foreground (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));

    cairo_new_sub_path(cr);

    cairo_arc (cr, width / 2.0, height / 2.0,
	       rp -> radius + ID,
	       range_start,
	       range_end);

    cairo_line_to (cr, x1, y1);

    cairo_arc_negative (cr, width / 2.0, height / 2.0,
			rp -> radius + OD,
			range_end,
			range_start);  
  
    cairo_close_path (cr);
    cairo_fill (cr);
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

  if (NULL != p -> legend) {
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
    sprintf (buffer, "%s", p -> legend);
    show_text_left_justified (cr, 238 + XOFFSET, 85 + YOFFSET, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }

  // Print Maximum Value
  sprintf (buffer, "%.0f", p -> max);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - ((3 < strlen (buffer) ? 12 : 4)));
  show_text_left_justified (cr, 255 + XOFFSET, 248 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Minimum Value
    
  sprintf (buffer, "%.0f", p -> min);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - ((2 < strlen (buffer) ? 12 : 4)));
  show_text_left_justified (cr, 60 + XOFFSET, 165 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if (NULL != p -> label) {
    sprintf (buffer, "%s", p -> label);
    show_text_unjustified (cr, 70 + XOFFSET, 210 + YOFFSET, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }
    
  cairo_set_line_width (cr, 1.0);
  set_rgba_for_foreground (cr, get_warning_level (p -> value, p -> high_warn, p -> low_warn));

  // Print value field and burn-in 
  cairo_select_font_face (
			  cr,
			  "DSEG7Classic",
			  CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  sprintf (buffer, "%.0f", p -> value);
  
  if (3 < strlen (buffer)) {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 20);
    show_text_right_justified (cr,
			       99 + XOFFSET,
			       160 + YOFFSET, 
			       buffer,
			       4,
			       get_warning_level (p -> value, p -> high_warn, p -> low_warn),
			       true,
			       true);
  } else {
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 10);
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


static const struct PanelVTable radial_vtable = {
  .draw = draw_radial_gauge_panel
};


Panel* create_radial_gauge_panel (unsigned int row, unsigned int column) {
  RadialPanel *lg = calloc (1, sizeof *lg);
  lg -> base.draw = draw_radial_gauge_panel;
  lg -> base.vtable = &radial_vtable;
  lg -> base.row = row;
  lg -> base.column = column;

  // Default initializations follow.
  lg -> radius = DEFAULT_RADIUS;
  lg -> start_angle = DEFAULT_START_ANGLE;
  lg -> end_angle = DEFAULT_END_ANGLE;
  lg -> segment_count = DEFAULT_SEGMENT_COUNT;
  lg -> segment_gap_size = DEFAULT_SEGMENT_GAP_SIZE;

  if (NULL == lg -> arc_segments) {
    /* NULL means this cairo gauge has yet to be initialized and does not mean that an error has occured. */
    lg -> arc_segments = (arc_segment*) malloc (lg -> segment_count * sizeof (arc_segment));

    for (int i = 0; i < lg -> segment_count; i++) {
      double size_subarc_angle = (lg -> end_angle - lg -> start_angle) / lg -> segment_count;
      lg -> arc_segments[i].arc_start_angle = lg -> start_angle + i * size_subarc_angle;
      lg -> arc_segments[i].arc_end_angle = lg -> arc_segments[i].arc_start_angle
	+ size_subarc_angle - lg -> segment_gap_size;
    }
  }

  return (Panel*) lg;
}


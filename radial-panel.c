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
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;
  double offset;
  unit_type units;
  char label[64];
  char* output_format;
  
  double radius;
  double start_angle;
  double end_angle;

  int segment_count;
  arc_segment* arc_segments;
  double segment_gap_size;
} RadialPanel;
 

void
draw_radial_gauge_panel (GtkDrawingArea* area,
			 cairo_t* cr,
			 int width,
			 int height,
			 gpointer user_data)
{
  RadialPanel* rp = user_data;
  
  char buffer[80];
  
  assert (NULL != rp);

  double value = convert_units (rp -> value, rp -> units) + rp -> offset;
  int foreground_color = get_active_foreground_color (&rp -> base, value, rp -> high_warn, rp -> low_warn);
  int background_color = rp -> base.background_color;

  /*
   * Draw background, arc and segments
   */

  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);

  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, rp -> base.foreground_color, 0.9);
    rounded_rectangle (cr, 5.0, 5.0, height - 10, width - 10, 5.0);
    cairo_stroke (cr);
  }

  set_rgba (cr, foreground_color, 0.9);
  
  /* gauge arc */
  cairo_set_line_width (cr, 3.0);
  cairo_arc (cr, width / 2.0, height / 2.0,
	     rp -> radius + 10,
	     rp -> start_angle,
	     rp -> end_angle);
  cairo_stroke (cr);
  
  /* illuminated segments */
  cairo_set_line_width (cr, 7.0);

  double t = CLAMP ((value - rp -> min) / (rp -> max - rp -> min), 0.0, 1.0);
  double angle = rp -> start_angle + t * (rp -> end_angle
					  - rp -> start_angle);

  for (int i = 0; i < rp -> segment_count; i++) {
    if (angle < rp -> arc_segments[i].arc_end_angle) {
      set_rgba (cr, foreground_color, 0.14);
    }
    else {
      set_rgba (cr, foreground_color, 0.9);
    }

    cairo_arc (cr, width / 2.0, height / 2.0,
	       rp -> radius + 2,
	       rp -> arc_segments[i].arc_start_angle,
	       rp -> arc_segments[i].arc_end_angle);

    cairo_stroke (cr);
  }

  #define ID 11
  #define OD 18

  if ((!isnan (rp -> low_warn)) && (!isnan (rp -> high_warn)) && (!isnan (rp -> min)) && (!isnan (rp -> max))) {

    t = CLAMP ((rp -> low_warn - rp -> min) / (rp -> max - rp -> min), 0.0, 1.0);
    double range_start = rp -> start_angle + t * (rp -> end_angle -  rp-> start_angle);

    t = CLAMP ((rp -> high_warn - rp -> min) / (rp -> max - rp -> min), 0.0, 1.0);
    double range_end = rp -> start_angle + t * (rp -> end_angle - rp -> start_angle);

    double x1 = width / 2 + (rp -> radius + ID) * cos (range_end);
    double y1 = height / 2 + (rp -> radius + ID) * sin (range_end);
    
    set_rgba (cr, foreground_color, 0.9);
    
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
  cairo_select_font_face (cr,
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 4);
  sprintf (buffer, "%s", str_from_unit_enum (panel_get_units (&rp -> base)));
  show_text_left_justified (cr, 238 + XOFFSET, 85 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Maximum Value
  sprintf (buffer, "%.0f", rp -> max);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - ((3 < strlen (buffer) ? 12 : 4)));
  show_text_left_justified (cr, 255 + XOFFSET, 248 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Minimum Value
  sprintf (buffer, "%.0f", rp -> min);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - ((2 < strlen (buffer) ? 12 : 4)));
  show_text_left_justified (cr, 60 + XOFFSET, 165 + YOFFSET, buffer);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);

  // Print Label
  if ('\0' != rp -> label[0]) {
    sprintf (buffer, "%s", rp -> label);
    show_text_unjustified (cr, 70 + XOFFSET, 210 + YOFFSET, buffer);
    cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  }
    
  cairo_set_line_width (cr, 1.0);
  
  // Print value field and burn-in 
  cairo_select_font_face (
			  cr,
			  "DSEG7Classic",
			  CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  sprintf (buffer, rp -> output_format, value);
  
  int w = strlen (buffer);
  switch (w) {
  case 0:
  case 1:
  case 2:
  case 3:
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 10);
    show_text_right_justified (cr,
			       width / 2.0,
			       160 + YOFFSET, 
			       buffer,
			       3,
			       foreground_color,
			       true,
			       true);
    break;
    
  case 4:
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 20);
    show_text_right_justified (cr,
			       width / 2.0,
			       160 + YOFFSET, 
			       buffer,
			       (NULL == strchr (buffer, '.')) ? 4 : 3,
			       foreground_color,
			       true,
			       true);
    break;
			       
  default:
    cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 20);

    if (NULL != strchr (buffer, '.')) {
      buffer[strlen (buffer) - 1] = '\0';
    }

    if (NULL != strchr (buffer, '.')) {
      buffer[strlen (buffer) - 1] = '\0';
    }
    
    show_text_right_justified (cr,
			       width / 2.0,
			       160 + YOFFSET, 
			       buffer,
			       strlen (buffer),
			       foreground_color,
			       true,
			       true);
    break;
  }
}

static double get_min (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> min); }
static double get_max (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> max); }
static double get_high_warn (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> high_warn); }
static double get_low_warn (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> low_warn); }
static double get_offset (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> offset); }
static unit_type get_units (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> units); }
static char* get_label (const Panel* g) { RadialPanel* rp = (RadialPanel*) g; return (rp -> label); }

static void set_minmax (Panel* g, double min, double max) { RadialPanel* rp = (RadialPanel*) g; rp -> min = min; rp -> max = max; }
static void set_warn (Panel* g, double low, double high) { RadialPanel* rp = (RadialPanel*) g; rp -> low_warn = low; rp -> high_warn = high; }
static void set_offset (Panel* g, double offset)  { RadialPanel* rp = (RadialPanel*) g; rp -> offset = offset; }
static void set_units (Panel* g, unit_type ut)  { RadialPanel* rp = (RadialPanel*) g; rp -> units = ut; }
static void set_label (Panel* g, char* label) { RadialPanel* rp = (RadialPanel*) g; strcpy (rp -> label, label); }
static void set_value (Panel* g, double value, int sensor_count) { RadialPanel* rp = (RadialPanel*) g; rp -> value = value; }
static void set_output_format (Panel* g, char* format) { RadialPanel* rp = (RadialPanel*) g; rp -> output_format = strdup (format); }

static const struct PanelVTable radial_vtable = {
  .draw = (void (*)(const struct Panel*, void *))draw_radial_gauge_panel,  
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
  .set_value = (void (*) (Panel*, double, int)) set_value,
  .set_output_format = (void (*) (Panel*, char*)) set_output_format
};

Panel* create_radial_gauge_panel (int x_index, int y_index, int z_index, double max, double min) {
  RadialPanel *lg = g_new0 (typeof (*lg), 1);

  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_radial_gauge_panel;
  lg -> base.vtable = &radial_vtable;
  lg -> base.x_index = x_index;
  lg -> base.y_index = y_index;
  lg -> base.z_index = z_index;

  lg -> base.background_color = XBLACK_RGB;
  lg -> base.foreground_color = XORANGE_RGB;
  lg -> base.high_warn_color = XRED_RGB;
  lg -> base.low_warn_color = XBLUE_RGB;
    
  lg -> output_format = "%.0f";
  lg -> max = max;
  lg -> min = min;
  lg -> radius = DEFAULT_RADIUS;
  lg -> start_angle = DEFAULT_START_ANGLE;
  lg -> end_angle = DEFAULT_END_ANGLE;
  lg -> segment_count = DEFAULT_SEGMENT_COUNT;
  lg -> segment_gap_size = DEFAULT_SEGMENT_GAP_SIZE;

  lg -> arc_segments = (arc_segment*) malloc (lg -> segment_count * sizeof (arc_segment));

  for (int i = 0; i < lg -> segment_count; i++) {
    double size_subarc_angle = (lg -> end_angle - lg -> start_angle) / lg -> segment_count;
    lg -> arc_segments[i].arc_start_angle = lg -> start_angle + i * size_subarc_angle;
    lg -> arc_segments[i].arc_end_angle = lg -> arc_segments[i].arc_start_angle
      + size_subarc_angle - lg -> segment_gap_size;
  }

  return (Panel*) lg;
}


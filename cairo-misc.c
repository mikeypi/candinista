#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "cairo_panel.h"

warning_level
get_warning_level (double value, double high_warn, double low_warn) {
  if (!isnan (low_warn) && (value < low_warn)) {
    return (LOW_WARN);
  }
  if (!isnan (high_warn) && (value > high_warn)) {
    return (HIGH_WARN);
  }

  return (NO_WARN);
}

void
set_rgba_for_burn_in (cairo_t* cr, enum warning_level wl) {
  switch (wl) {
  case HIGH_WARN:
    cairo_set_source_rgba (cr, HIGH_WARN_BURN_IN_RGBA);
    break;
  case LOW_WARN:
    cairo_set_source_rgba (cr, LOW_WARN_BURN_IN_RGBA);
    break;
  case NO_WARN:
    cairo_set_source_rgba (cr, BURN_IN_RGBA);
    break;
  }
}

void
set_rgba_for_foreground  (cairo_t* cr, enum warning_level wl) {
  switch (wl) {
  case HIGH_WARN:
    cairo_set_source_rgba (cr, HIGH_WARN_RGBA);
    break;
  case LOW_WARN:
    cairo_set_source_rgba (cr, LOW_WARN_RGBA);
    break;
  case NO_WARN:
    cairo_set_source_rgba (cr, FOREGROUND_RGBA);
    break;
  }
}

void
set_rgba_for_background (cairo_t* cr) {
  cairo_set_source_rgba (cr, BACKGROUND_RGBA);
}

void
show_text_unjustified (cairo_t* cr, int x, int y, char* buffer) {
  cairo_move_to (cr, x, y);
  cairo_show_text (cr, buffer);
}

void
show_text_left_justified (cairo_t* cr, int x, int y, char* buffer) {
  cairo_text_extents_t extents;

  cairo_text_extents(cr, buffer, &extents);
  cairo_move_to (cr, x - extents.width, y);
  cairo_show_text (cr, buffer);
}

int
show_text_right_justified (cairo_t* cr,
			   int x,
			   int y,
			   char* buffer,
			   int width,
			   warning_level warn,
			   bool burn_in,
			   bool add_box) {

  cairo_text_extents_t extents;
  char* ghost_chars[] = {"8", "88", "888", "8888", "88888", "888888"};
    
  switch (width) {
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    cairo_text_extents(cr, ghost_chars[width - 1], &extents);
  break;
  
  default:
    return (-1);
  }

#define BOX_HEIGHT_MARGIN 7
#define BOX_WIDTH_MARGIN 5
  
  if (add_box) {
    set_rgba_for_foreground (cr, warn);
    rounded_rectangle(cr,
		      x - (BOX_WIDTH_MARGIN / 2),
		      y - (extents.height + BOX_HEIGHT_MARGIN),
		      extents.width + BOX_WIDTH_MARGIN * 2,
		      extents.height + BOX_HEIGHT_MARGIN * 2,
		      5.0);
    cairo_stroke (cr);
  }

  double start_x = x + ((extents.x_advance / width) * (width - strlen (buffer)));

  if (burn_in) {
    set_rgba_for_burn_in (cr, warn);
    cairo_move_to (cr, x , y);
    cairo_show_text (cr, ghost_chars[width - 1]);
  }
  
  set_rgba_for_foreground (cr, warn);
  cairo_move_to (cr, start_x , y);
  cairo_show_text (cr, buffer);
}

void
rounded_rectangle (cairo_t* cr,
		   double x,
		   double y,
		   double width,
		   double height,
		   double radius) {

  double r = radius;

  /* Clamp radius so it doesn't exceed half the size */
  if (r > width / 2)  r = width / 2;
  if (r > height / 2) r = height / 2;

  cairo_new_sub_path(cr);

  /* Top-right corner */
  cairo_arc(cr,
	    x + width - r, y + r,
	    r,
	    -M_PI / 2, 0);

  /* Bottom-right corner */
  cairo_arc(cr,
	    x + width - r, y + height - r,
	    r,
	    0, M_PI / 2);

  /* Bottom-left corner */
  cairo_arc(cr,
	    x + r, y + height - r,
	    r,
	    M_PI / 2, M_PI);

  /* Top-left corner */
  cairo_arc(cr,
	    x + r, y + r,
	    r,
	    M_PI, 3 * M_PI / 2);

  cairo_close_path(cr);
}

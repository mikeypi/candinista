#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "cairo-misc.h"

void
set_rgba (cairo_t* cr, unsigned int color, double alpha) {
  unsigned int red =   (color >> 16) & 0xff;
  unsigned int green = (color >> 8) & 0xff;
  unsigned int blue =  (color) & 0xff;

  cairo_set_source_rgba (cr, (double)red/255.0, (double)green/255.0, (double)blue/255.0, alpha);
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

#define BOX_HEIGHT_MARGIN 5
#define BOX_WIDTH_MARGIN 5

int
show_text_right_justified (cairo_t* cr,
			   int x,
			   int y,
			   char* buffer,
			   int width,
			   unsigned int color,
			   bool burn_in,
			   bool add_box) {

  cairo_text_extents_t ghost_extents;
  cairo_text_extents_t buffer_extents;
  char* ghost_chars[] = {"8", "88", "888", "8888", "88888", "888888"};
    
  switch (width) {
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    cairo_text_extents (cr, ghost_chars[width - 1], &ghost_extents);
    cairo_text_extents (cr, buffer, &buffer_extents);
    break;
  
  default:
    fprintf (stderr, "unsupported width\n");
    return (-1);
  }

  double field_width = (buffer_extents.width < ghost_extents.width) ?  ghost_extents.x_advance : buffer_extents.x_advance;
  double field_height = ghost_extents.height;  
  double right_margin = x + (field_width / 2.0);
  double left_margin = right_margin - field_width;
  double buffer_left_margin = right_margin - buffer_extents.x_advance;
  
  if (add_box) {
    set_rgba (cr, color, 0.9);
    rounded_rectangle(cr,
		      left_margin - BOX_WIDTH_MARGIN,
		      y - field_height - BOX_HEIGHT_MARGIN,
		      field_height + (2 * BOX_HEIGHT_MARGIN),
		      field_width + (2 * BOX_WIDTH_MARGIN),
		      5.0);

    cairo_stroke (cr);
  }

  if (burn_in) {
    set_rgba (cr, color, 0.14);
    cairo_move_to (cr,left_margin, y);

    cairo_show_text (cr, ghost_chars[width - 1]);
  }
  
  set_rgba (cr, color, 0.9);
  cairo_move_to (cr, buffer_left_margin, y);
  cairo_show_text (cr, buffer);

  return (0);
}

void
rounded_rectangle (cairo_t* cr,
		   double x,
		   double y,
		   double height,
		   double width,
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

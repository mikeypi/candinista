#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "cairo_panel.h"


cairo_info_panel* new_cairo_info_panel () {
  cairo_info_panel* g = g_new0 (cairo_info_panel, 1);
  return (g);
}

void draw_cairo_info_panel (GtkDrawingArea *area,
		       cairo_t *cr,
		       int width,
		       int height,
		       gpointer user_data)
{
  cairo_gauge_panel* gd = user_data;
  
  char buffer[80];
  
  if (NULL == gd) {
    fprintf (stderr, "NULL gd\n");
    return;
  }

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
  cairo_set_source_rgba (cr, FOREGROUND_RGBA);

  if (gd -> border) {
    cairo_set_line_width (cr, 2.0);
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
			  CAIRO_FONT_WEIGHT_BOLD
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 14);
  cairo_set_source_rgba (cr, FOREGROUND_RGBA);

#define XX 20
#define YY 40
  
  time_t timer = time (NULL);
  sprintf (buffer, "%s", ctime (&timer));
  /* the string returned by ctime ends with a newline. This fixes that. */
  buffer [strlen (buffer) - 1] = '\0';
  show_text_unjustified (cr, XX, YY, buffer);  

  FILE *f = fopen("/proc/loadavg", "r");
  double a, b, c;

  if (f && fscanf(f, "%lf %lf %lf", &a, &b, &c) == 3) {
    sprintf( buffer, "Load avg: %.2f %.2f %.2f", a, b, c);
    fclose(f);
  }
  
  show_text_unjustified (cr, XX, YY+23, buffer);

  f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
  int temp;
  fscanf(f, "%d", &temp);
  fclose(f);

  sprintf(buffer, "CPU temp: %.1f°C", temp / 1000.0);
  show_text_unjustified (cr, XX, YY+46, buffer);
  
  cairo_surface_destroy (surface);
}

#define XOFFSET 0
#define YOFFSET 40

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include <sys/sysinfo.h>

#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"


/* concrete type */
typedef struct {
  Panel base;
  /* layout-specific fields could go here */
} InfoPanel;


void draw_info_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  InfoPanel* rp = user_data;
  Panel* p = user_data;
  char buffer[80];
  int i;

  if (NULL == rp) {
    return;
  }

  double value = convert_units (p -> value, p -> units) + p -> offset;

  set_rgba_for_background (cr);
  cairo_paint (cr);
  
  if (0 != p -> border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba_for_foreground (cr, get_warning_level (value, p -> high_warn, p -> low_warn));
    rounded_rectangle(cr, 5.0, 5.0, width - 10, height - 10, 5.0);
    cairo_stroke (cr);
  }

  set_rgba_for_foreground (cr, get_warning_level (value, p -> high_warn, p -> low_warn));
  
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

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 14);
  set_rgba_for_foreground (cr, get_warning_level (value, p -> high_warn, p -> low_warn));

  time_t timer = time (NULL);
  sprintf (buffer, "%s", ctime (&timer));
  /* the string returned by ctime ends with a newline. This fixes that. */
  buffer [strlen (buffer) - 1] = '\0';
  show_text_unjustified (cr, 20 + XOFFSET, 5 + YOFFSET, buffer);


  FILE *f = fopen("/proc/loadavg", "r");
  double a, b, c;

  if (f && fscanf(f, "%lf %lf %lf", &a, &b, &c) == 3) {
    sprintf( buffer, "Load avg: %.2f %.2f %.2f", a, b, c);
    fclose(f);
  }

  show_text_unjustified (cr, 20 + XOFFSET, 28 + YOFFSET, buffer);

  f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
  int temp;
  fscanf(f, "%d", &temp);
  fclose(f);

  sprintf(buffer, "CPU temp: %.1f°C", temp / 1000.0);
  show_text_unjustified (cr, 20 + XOFFSET, 51 + YOFFSET, buffer);

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE);
  cairo_surface_destroy (surface);
}


static const struct PanelVTable linear_vtable = {
  .draw = (void (*)(const struct Panel *, void *))draw_info_panel
};


Panel* create_info_panel (
			  unsigned int x_index,
			  unsigned int y_index,
			  unsigned int z_index
			  ) {

  InfoPanel *lg = calloc (1, sizeof *lg);
  int i;
  
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_info_panel;
  lg -> base.vtable = &linear_vtable;
  lg -> base.x_index = x_index;
  lg -> base.y_index = y_index;
  lg -> base.z_index = z_index;

  return (Panel*) lg;
}


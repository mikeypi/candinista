#define XOFFSET 0
#define YOFFSET 40

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include <sys/sysinfo.h>

#include "units.h"
#include "candinista.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"


/* concrete type */
typedef struct {
  Panel base;
} InfoPanel;


void draw_info_panel (GtkDrawingArea* area,
				cairo_t* cr,
				int width,
				int height,
				gpointer user_data)
{
  InfoPanel* rp = user_data;
  char buffer[80];

  if (NULL == rp) {
    return;
  }

  int foreground_color = rp -> base.foreground_color;
  int background_color = rp -> base.background_color;
  
  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);
  
  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, foreground_color, 0.9);    
    rounded_rectangle (cr, 5.0, 5.0, height - 10, width - 10, 5.0);
    cairo_stroke (cr);
  }

  set_rgba (cr, foreground_color, 0.9);
  
  /*
   * Draw labels and current value
   */
  cairo_select_font_face (
			  cr,
			  "Orbitron",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL
			  );

  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 12);
  set_rgba (cr, foreground_color, 0.9);

  time_t timer = time (NULL);
  sprintf (buffer, "%s", ctime (&timer));
  /* the string returned by ctime ends with a newline. This fixes that. */
  buffer [strlen (buffer) - 1] = '\0';
  show_text_unjustified (cr, 20 + XOFFSET, 5 + YOFFSET, buffer);


  FILE *f = fopen ("/proc/loadavg", "r");
  double a, b, c;

  if (f && fscanf (f, "%lf %lf %lf", &a, &b, &c) == 3) {
    sprintf ( buffer, "Load avg: %.2f %.2f %.2f", a, b, c);
    fclose (f);
  }

  show_text_unjustified (cr, 20 + XOFFSET, 28 + YOFFSET, buffer);

  f = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
  int temp;
  fscanf (f, "%d", &temp);
  fclose (f);

  sprintf (buffer, "CPU temp: %.1f°C", temp / 1000.0);
  show_text_unjustified (cr, 20 + XOFFSET, 51 + YOFFSET, buffer);
}


void print_info_panel (const Panel* g)
{
}

static void set_value (Panel* g, double value, int sensor_offfset, int can_id) {}

static const struct PanelVTable info_vtable = {
  .draw = (void (*)(const struct Panel *, void *))draw_info_panel,
  .print = (void (*) (const Panel*)) print_info_panel,
  .set_value = (void (*) (Panel*, double, int, int)) set_value,
};


Panel* create_info_panel (PanelParameters* p) {

  InfoPanel *lg = calloc (1, sizeof *lg);
  
  lg = (InfoPanel*) panel_init_base (p, (Panel*) lg);
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_info_panel;
  lg -> base.vtable = &info_vtable;
  
  return (Panel*) lg;
}


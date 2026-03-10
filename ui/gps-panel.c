#define XOFFSET 0
#define YOFFSET 40

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <math.h>
#include <assert.h>

#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "cairo-misc.h"
#include "gtk-glue.h"
#include "candinista.h"
#include "panel_specs.h"
 
void draw_gps_panel (GtkDrawingArea* area,
		      cairo_t* cr,
		      int width,
		      int height,
		      gpointer user_data)
{
  (void) area;
  
  GPSPanel* rp = user_data;
  assert (NULL != rp);

  int background_color = rp -> base.background_color;
  
  double xx;
  double yy;
  char abuf[80];
  
  set_rgba (cr, background_color, 1.0);
  cairo_paint (cr);

  if (0 != rp -> base.border) {
    cairo_set_line_width (cr, 1.0);
    set_rgba (cr, rp -> base.foreground_color, 1.0);
    rounded_rectangle (cr, 5.0, 5.0, height - 10, width - 10, 5.0);
    cairo_stroke (cr);
  }

  cairo_select_font_face (cr, "DSEG7Classic", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);

  cairo_set_font_size (cr, DEFAULT_VALUE_FONT_SIZE - 45);
  cairo_set_line_width (cr, 1.0);

  const double delta_y = 36;

  xx = 50;
  yy = 43;
  
  sprintf (abuf, "%.1f", rp -> lattitude);
  show_text_right_justified (cr, xx, yy, abuf, 5);
  yy += delta_y;
  
  sprintf (abuf, "%.1f", rp -> longitude);
  show_text_right_justified (cr, xx, yy, abuf, 5);
  yy += delta_y;
  
  sprintf (abuf, "%.1f", rp -> speed);
  show_text_right_justified (cr, xx, yy, abuf, 5);
  yy += delta_y;
  
  sprintf (abuf, "%.1f", rp -> heading_motion);
  show_text_right_justified (cr, xx, yy, abuf, 5);
  yy += delta_y;
  
  sprintf (abuf, "%.1f", rp -> vehicle_motion);
  show_text_right_justified (cr, xx, yy, abuf, 5);
  yy += delta_y;

  sprintf (abuf, "%.1f", rp -> altitude);
  show_text_right_justified (cr, xx, yy, abuf, 5);

  yy += delta_y;
  sprintf (abuf, "%02i-%02i-%d  %02i:%02i:%02i", rp -> utc_day, rp -> utc_month, rp -> utc_year,
	   rp -> utc_hour, rp -> utc_minute, rp -> utc_second);
  
  show_text_left_justified (cr, xx + 260, yy + 20, abuf);
  yy += delta_y;

  xx = 210;
  yy = 43;

  sprintf (abuf, "%.1f", rp -> x_acceleration);
  show_text_right_justified (cr, xx, yy, abuf, 4);
  yy += delta_y;

  sprintf (abuf, "%.1f", rp -> x_acceleration);
  show_text_right_justified (cr, xx, yy, abuf, 4);
  yy += delta_y;

  sprintf (abuf, "%.1f", rp -> x_acceleration);
  show_text_right_justified (cr, xx, yy, abuf, 4);
  yy += delta_y;

  sprintf (abuf, "%d", rp -> sat_count);
  show_text_right_justified (cr, xx, yy, abuf, 4);
  yy += delta_y;

  sprintf (abuf, "%d", rp -> gps_status);
  show_text_right_justified (cr, xx, yy, abuf, 4);
  yy += delta_y;

  cairo_select_font_face (cr, "Orbitron", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, DEFAULT_LABEL_FONT_SIZE - 14);

  char* labels[] = {"lat", "long", "speed", "h-motion", "v-motion", "alt", "x-accel", "y-accel", "z-accel", "sat count", "gps"};

  xx = 100;
  yy = 43;

  show_text_unjustified (cr, xx, yy, labels[0]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[1]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[2]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[3]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[4]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[5]);
  yy += delta_y;

  xx = 250;
  yy = 43;
  
  show_text_unjustified (cr, xx, yy, labels[6]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[7]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[8]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[9]);
  yy += delta_y;
  show_text_unjustified (cr, xx, yy, labels[10]);
  yy += delta_y;
    
  cairo_stroke (cr);
}


static void set_value (Panel* g, double value, int sensor_offset, int can_id) {
  GPSPanel* rp = (GPSPanel*) g;
  switch (can_id) {
  case 0x400:
    if (0 == sensor_offset) {
      rp -> lattitude = value;
      return;
    }
    if (1 == sensor_offset) {
      rp -> longitude = value;
      return;
    }

    break;
    
  case 0x401:
    if (0 == sensor_offset) {
      rp -> speed = value;
      return;
    }
    if (1 == sensor_offset) {
      rp -> altitude = value;
      return;
    }
    if (2 == sensor_offset) {
      rp -> sat_count = value;
      return;
    }
    if (3 == sensor_offset) {
      rp -> gps_status = value;
      rp -> gps_status &= 0x7;
      return;
    }

    break;
    
  case 0x402:
    if (0 == sensor_offset) {
      rp -> heading_motion = value;
      return;
    }
    if (1 == sensor_offset) {
      rp -> heading_motion = value;
      return;
    }

    break;

  case 0x403:
    if (0 == sensor_offset) {
      rp -> x_acceleration = value;
      return;
    }
    if (1 == sensor_offset) {
      rp -> y_acceleration = value;
      return;
    }
    if (2 == sensor_offset) {
      rp -> z_acceleration = value;
      return;
    }

    break;

  case 0x404:
    if (0 == sensor_offset) {
      rp -> utc_year = value;
      return;
    }
    if (1 == sensor_offset) {
      rp -> utc_month = value;
      return;
    }
    if (2 == sensor_offset) {
      rp -> utc_day = value;
      return;
    }
    if (3 == sensor_offset) {
      rp -> utc_hour = value;
      return;
    }
    if (4 == sensor_offset) {
      rp -> utc_minute = value;
      return;
    }
    if (5 == sensor_offset) {
      rp -> utc_second = value;
      return;
    }

    break;
  }

  fprintf (stderr, "Bad can_id = %x / sensor_offset = %d\n", can_id, sensor_offset);  
}

void print_gps_panel (FILE* fp, const Panel* g)
{
  (void) fp;
  (void) g;
}

static const struct PanelVTable gps_vtable = {
  .draw = (void (*)(void *, cairo_t*, int, int, void*)) draw_gps_panel,
  .print = (void (*) (FILE* fp, const Panel*)) print_gps_panel,
  .set_value = (void (*) (Panel*, double, int, int)) set_value,
};

Panel* create_gps_panel (PanelParameters* p) {
  GPSPanel *lg = g_new0 (typeof (*lg), 1);
  
  lg = (GPSPanel*) panel_init_base (p, (Panel*) lg);
  lg -> base.draw = (void (*)(void*, cairo_t*, int, int, void*))draw_gps_panel;
  lg -> base.vtable = &gps_vtable;

  return (Panel*) lg;
}

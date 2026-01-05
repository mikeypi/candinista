/*
 * Copyright (c) 2024, Joseph Hollinger
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>

#include <fontconfig/fontconfig.h>

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <sys/time.h>

#include "candinista.h"
#include "cairo_panel.h"

#define nBytesToShort(a, b) ((a << 8) | b)

static short
BytesToShort (char a, char b) {
  unsigned short x = (a << 8) ^ b;
  return (x);
}

static float
convert_units (float temp, unit_type to) {
  switch (to) {
  case FAHRENHEIT:
    return ((temp * 9.0 / 5.0) + 32.0);

  case PSI:
    return (temp * 14.503773773);

  case CELSIUS:
  case BAR:
  default:
    return (temp);
  }
}

int
timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

static void
update_widgets_for_display (output_descriptor* od, double f) {

  switch (od -> type) {
  case CAIRO_INFO_PANEL:
    return;
    
    case CAIRO_BARGRAPH_PANEL:
      cairo_gauge_panel* cbg = (cairo_gauge_panel*) od -> output;

      if (NULL == cbg) {
	return;
      }

      cbg -> value = f + od -> offset;
      return;

  case CAIRO_GAUGE_PANEL:
    default:
      cairo_gauge_panel* cgp = (cairo_gauge_panel*) od -> output;

      if (NULL == cgp) {
	return;
      }

      cgp -> value = f + od -> offset;
      return;
  }
}

//extern output_descriptor* output_descriptor_by_name (char*);

static gboolean
idle_task () {
  return TRUE;
}

/*
 * Called when there is can data ready to be read. Read it from the frame, interpolate and convert if required
 * and update the corresponding display widgets.
 */
static gboolean
can_data_ready_task (GIOChannel* input_channel, GIOCondition condition, gpointer data)
{
  int i;
  static int call_count;
  double temp;
  struct can_frame frame;
  gsize bytes_read;

  sensor_descriptor* s = sensor_descriptors;
  if (G_IO_STATUS_NORMAL != g_io_channel_read_chars (input_channel,
						     (gchar*) &frame, sizeof (struct can_frame),
						     &bytes_read, NULL)) {
    return FALSE;
  }

  while (s < sensor_descriptors + sensor_count) {
    if (s -> can_id != (frame.can_id & 0x7fffffff)) {
      s++;
      continue;
    }

    /* retrieve the individual data values from the can frame. This handles char and short,
     * would need to be expanded for 32 or 64 bit data
     */
    if (sizeof (short) == s -> can_data_width) {
      temp = BytesToShort (frame.data[s -> can_data_offset],frame.data[s -> can_data_offset + 1]);
    } else {
      temp = frame.data[s -> can_data_offset];
    }
  
    /* apply interpolation if needed */
    temp = linear_interpolate (temp, s);

    if (NULL != s -> output_descriptor) {
      temp = convert_units (temp, s -> output_descriptor -> units);
      update_widgets_for_display (s -> output_descriptor, temp);
    }

    if (0 != data_logging) {
      log_data (&frame);
    }

    s++;
  }

  return TRUE;
}

/*
 * Build the GTK GUI and associate the output widgets with the appropriate data struture so that their values
 * can be updated dynamically.
 */

static void
activate (GtkApplication* app,
          gpointer        user_data) {

  GtkBuilder* builder;
  GObject* window;
  GtkCssProvider* provider;
  output_descriptor* od = output_descriptors;
  
  ui_file_name = "/home/joe/candinista/candinista.ui";
  if (NULL == (builder = gtk_builder_new_from_file (ui_file_name))) {
    char temp[PATH_MAX];
    fprintf (stderr, "could not open UI configuration file %s in %s\n", ui_file_name, getcwd (temp, sizeof(temp)));
    exit (-1);
  }

  window = gtk_builder_get_object (builder, "window");

  if (0 == remote_display) {
    gtk_window_fullscreen (GTK_WINDOW(window));
  }

  gtk_window_set_application (GTK_WINDOW (window), app);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_file (provider, g_file_new_for_path (CSS_FILE_NAME));
  
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
      					      GTK_STYLE_PROVIDER (provider),
      					      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  GtkGrid* grid = (GtkGrid*) gtk_builder_get_object (builder, "grid-0");
  if (NULL == grid) {
    fprintf (stderr, "unable to load grid\n");
    return;
  }

  while (od < output_descriptors + output_count) {
    char temp[80];

    sprintf (temp, "da-%d-%d", od -> row, od -> column);

    GtkDrawingArea* drawing_area = (GtkDrawingArea*) gtk_builder_get_object (builder, temp);

    if (NULL == drawing_area) {
      fprintf (stderr, "unable to load drawing_area %s at %d, %d\n", temp, od -> row, od -> column);
      return;
    }

    fprintf (stderr, "drawing_area %s loaded at %d, %d\n", temp, od -> row, od -> column);

    switch (od -> type) {
    case CAIRO_INFO_PANEL:
      cairo_info_panel* cip = new_cairo_info_panel ();

      cip -> border = od -> border;

      struct {
	GtkDrawingArea* drawing_area;
	void* cg;
      }* ctx = g_new0 (typeof (*ctx), 1);

      ctx -> drawing_area = drawing_area;
      ctx -> cg = cip;

      gtk_drawing_area_set_draw_func (drawing_area, draw_cairo_info_panel, cip, NULL);
      g_timeout_add (750, update_cairo_gauge_panel_value, ctx);

      od -> output = (void*) cip;
      break;

    case CAIRO_BARGRAPH_PANEL:
      cairo_bargraph_panel* cbp = new_cairo_bargraph_panel ();

      cbp -> max = od -> max;
      cbp -> min = od -> min;
      cbp -> low_warn = od -> low_warn;
      cbp -> high_warn = od -> high_warn;
      cbp -> label = od -> label;
      cbp -> legend = od -> legend;
      cbp -> border = od -> border;

      ctx = g_new0 (typeof (*ctx), 1);

      ctx -> drawing_area = drawing_area;
      ctx -> cg = cbp;

      gtk_drawing_area_set_draw_func (drawing_area, draw_cairo_bargraph_panel, cbp, NULL);
      g_timeout_add (500, update_cairo_bargraph_panel_value, ctx);

      od -> output = (void*) cbp;
      break;

    case CAIRO_GAUGE_PANEL:
    default:
      cairo_gauge_panel* cgp = new_cairo_gauge_panel ();
    
      cgp -> min = od -> min;
      cgp -> max = od -> max;
      cgp -> low_warn = od -> low_warn;
      cgp -> high_warn = od -> high_warn;
      cgp -> label = od -> label;
      cgp -> legend = od -> legend;
      cgp -> border = od -> border;
      
      ctx = g_new0 (typeof (*ctx), 1);
      ctx -> drawing_area = drawing_area;
      ctx -> cg = cgp;

      gtk_drawing_area_set_draw_func (drawing_area, draw_cairo_gauge_panel, cgp, NULL);
      g_timeout_add (500, update_cairo_gauge_panel_value, ctx);

      od -> output = (void*) cgp;
      break;
    }
    od++;
  }

  g_object_unref (builder);
  gtk_window_present (GTK_WINDOW (window));
}

/*
 * Build a socket to read can bus data.
 */
static GIOChannel*
can_setup () {
  int s;
  struct sockaddr_can addr;
  struct ifreq ifr;
  GIOChannel* input_channel;
    
  if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
    return NULL;
  }

  strcpy(ifr.ifr_name, can_socket_name);
  ioctl(s, SIOCGIFINDEX, &ifr);
	
  addr.can_family  = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    return NULL;
  }
  
  input_channel = g_io_channel_unix_new (s);
  g_io_channel_set_encoding (input_channel, NULL, NULL);
  return input_channel;
}

int
main (int argc, char** argv) {
  GtkApplication *app;
  GIOChannel* input_channel;
  int status;
  int i;
  int total;
  int option;

  get_environment_variables ();

  read_config_from_json ();
  
  while (-1 != (option = getopt (argc, argv, "dp"))) {
    switch (option) {
    case 'd':
      data_logging = 1;
      break;
      
    case 'p':
      print_config (stderr);
      exit (0);
      break;

    default:
      fprintf (stderr, "unknown option %c\n", option);
      fprintf (stderr,
	       "Usage: %s options device_name.\n"
	       "Options:\n"
	       "\t -n: disable datalogging\n"
	       "\t -p: print json database\n",
	       argv[0]);
      exit (-1);
    }
  }

  if (NULL == (input_channel = can_setup ())) {
    perror("Error in socket bind");
    return -1;
  }

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_NON_UNIQUE);
  
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_io_add_watch (input_channel, G_IO_IN, can_data_ready_task, NULL);
  g_idle_add (idle_task, NULL);

  /* not sure why this is required, but g_application_run will throw an error if it is called with
   * additional flags in argv (e.g., -n).
   */
  argv[1] = NULL;
  argc = 1;
  status = g_application_run (G_APPLICATION (app), argc, argv);
  
  g_object_unref (app);

  return status;
}

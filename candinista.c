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
#include <assert.h>
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
#include "cairo-misc.h"
#include "units.h"
#include "yaml-loader.h"
#include "yaml-printer.h"
#include "sensor.h"
#include "panel.h"
#include "gtk-glue.h"
#include "datalogging.h"


Configuration cfg;

static short
BytesToShort (unsigned char a, unsigned char b) {
  unsigned short x = (a << 8) ^ b;
  return (x);
}

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
  int i = 0;
  static int call_count;
  double temp;
  struct can_frame frame;
  gsize bytes_read;

  if (G_IO_STATUS_NORMAL != g_io_channel_read_chars (input_channel,
						     (gchar*) &frame, sizeof (struct can_frame),
						     &bytes_read, NULL)) {
    return FALSE;
  }

  while (i < cfg.sensor_count) {
    Sensor* s = cfg.sensors[i];
    if (sensor_get_can_id (s) != (frame.can_id & 0x7fffffff)) {
      i++;
      continue;
    }

    /*
     * retrieve the individual data values from the can frame. This handles char and short,
     * would need to be expanded for 32 or 64 bit data
     */
    int offset = sensor_get_can_data_offset (s);
    if (sizeof (short) == sensor_get_can_data_width (s)) {
      temp = BytesToShort (frame.data[offset], frame.data[offset + 1]);
    } else {
      temp = frame.data[offset];
    }

    if (0 > temp) {
      fprintf (stderr, "bad sensor data\n");
    }

    /* apply interpolation */
    temp = linear_interpolate (temp,
			       sensor_get_x_values (s),
			       sensor_get_y_values (s),
			       sensor_get_n_values (s));

    unsigned int x_index = sensor_get_x_index (s);
    unsigned int y_index = sensor_get_y_index (s);
    int z_index =  get_active_z (&cfg, x_index, y_index);

    Panel* p = cfg_get_panel (&cfg,
			      x_index,
			      y_index,
			      z_index);

    panel_set_value (p, temp);

    if (sensor_get_id (s) != panel_get_id (p)) {
      fprintf (stderr, "id mismatch %d != %d\n", sensor_get_id (s), panel_get_id (p));
    } 
      
    if (0 != data_logging) {
      log_data (&frame);
    }

    i++;
  }

  return TRUE;
}

/*
 * clicking a panel is supposed to cycle through all of the panels that have the same x, and
 * y but different z coordinates.
 */
static void
on_pressed(GtkGestureClick *gesture,
           int              n_press,
           double           x,
           double           y,
           gpointer         user_data)
{
  typedef struct {
    GtkDrawingArea* drawing_area;
    Panel* cg;
  } cx;

  cx* ctx = (cx*) user_data;

  unsigned int x_index = panel_get_x_index (ctx -> cg);
  unsigned int y_index = panel_get_y_index (ctx -> cg);
  unsigned int active_z = get_active_z (&cfg, x_index, y_index);

  Panel* p = NULL;
  
  for (int i = 1; i < cfg.z_dimension; i++) {
    int j = (active_z + i) % cfg.z_dimension;
    if (NULL != (p = cfg_get_panel (&cfg, x_index, y_index, j))) {
      set_active_z (&cfg, x_index, y_index, j);
      break;
    }
  }

  if (NULL != p) {
    ctx -> cg = p;
    gtk_drawing_area_set_draw_func (ctx -> drawing_area, gtk_draw_gauge_panel_cb, p, NULL);
    unsigned int timeout = panel_get_timeout (p);
    g_timeout_add ((0 == timeout) ? 600 : timeout, gtk_update_gauge_panel_value, ctx);
  }
}


static void
on_drawing_area_destroy (GtkWidget *widget, gpointer user_data)
{
  struct {
    GtkDrawingArea* drawing_area;
    Panel* cg;
    unsigned int timeout_id;
  }* ctx = user_data;

  if (0 != ctx -> timeout_id) {
    g_source_remove (ctx -> timeout_id);
  }
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
  Panel* p;
  
  ui_file_name = "/home/joe/candinista/candinista.ui";
  
  if (NULL == (builder = gtk_builder_new_from_file (ui_file_name))) {
    char temp[PATH_MAX];
    fprintf (stderr, "could not open UI configuration file %s in %s\n", ui_file_name, getcwd (temp, sizeof(temp)));
    exit (-1);
  }

  window = gtk_builder_get_object (builder, "window");
  
  gtk_window_set_application (GTK_WINDOW (window), app);

  if (0 == remote_display) {
    gtk_window_fullscreen (GTK_WINDOW(window));
  }

  GtkGrid* grid = (GtkGrid*) gtk_builder_get_object (builder, "grid-0");
  if (NULL == grid) {
    fprintf (stderr, "unable to load grid\n");
    return;
  }

  gtk_grid_set_column_homogeneous (grid, TRUE);

  g_object_unref (builder);
  gtk_window_present (GTK_WINDOW (window));

  for (int i = 0; i < cfg.y_dimension; i++) {
    for (int j = 0; j < cfg.x_dimension; j++) {
      GtkDrawingArea* drawing_area = (GtkDrawingArea*) gtk_drawing_area_new ();
      gtk_widget_set_hexpand (GTK_WIDGET(drawing_area), TRUE);
      gtk_widget_set_vexpand (GTK_WIDGET(drawing_area), TRUE);

      p = cfg_get_panel (&cfg, j, i, 0);
      assert (NULL != p);
	
      struct {
	GtkDrawingArea* drawing_area;
	Panel* cg;
	unsigned int timeout_id;
      }* ctx = g_new0 (typeof (*ctx), 1);

      unsigned int timeout = panel_get_timeout (p);

      ctx -> drawing_area = drawing_area;
      ctx -> cg = p;
      ctx -> timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT,
					      (timeout == 0) ? 600 : timeout,
					      gtk_update_gauge_panel_value,
					      ctx,
      					      g_free);

      g_signal_connect (drawing_area, "destroy",
                 G_CALLBACK (on_drawing_area_destroy), ctx);

      gtk_drawing_area_set_draw_func (drawing_area, gtk_draw_gauge_panel_cb, p, NULL);
      
      GtkGesture* click = gtk_gesture_click_new();
      g_signal_connect (click, "pressed",
			G_CALLBACK (on_pressed), ctx);

      gtk_widget_add_controller (GTK_WIDGET (drawing_area), GTK_EVENT_CONTROLLER (click));

      gtk_widget_set_size_request (GTK_WIDGET (drawing_area), 1024/3 - 1 , 600/2);  

      gtk_grid_attach(grid, GTK_WIDGET(drawing_area), j, i, 1, 1);
    }
  }
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

  strcpy (ifr.ifr_name, can_socket_name);
  ioctl (s, SIOCGIFINDEX, &ifr);
	
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
  int total;
  int option;

  get_environment_variables ();

  cfg = configuration_load_yaml ("/home/joe/candinista/config.yaml");
  build_tables (&cfg);

  while (-1 != (option = getopt (argc, argv, "dp"))) {
    switch (option) {
    case 'd':
      data_logging = 1;
      break;
      
    case 'p':
      configuration_print (&cfg);
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
    perror ("Error in socket bind");
    return -1;
  }

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_NON_UNIQUE);
  
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_io_add_watch (input_channel, G_IO_IN, can_data_ready_task, NULL);
  //g_idle_add (idle_task, NULL);

  /* not sure why this is required, but g_application_run will throw an error if it is called with
   * additional flags in argv (e.g., -n).
   */
  argv[1] = NULL;
  argc = 1;
  status = g_application_run (G_APPLICATION (app), argc, argv);

  
  g_object_unref (app);
  configuration_free (&cfg);
  
  return status;
}

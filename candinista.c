
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
update_widgets_for_display (output_descriptor* p, float f) {
  struct timeval now;
  struct timeval delta;

  if (0 != p-> update_interval) {
    gettimeofday (&now, NULL);
    timeval_subtract (&delta, &now, &p -> tv);
    if (delta.tv_sec <= p-> update_interval) {
      return;
    } else {
      p -> tv = now;
    }
  }

  if (0 != p-> update_floor) {
    if ((0 != p -> last_value) && (0 != f)) {
      if (abs (p -> last_value - f < p-> update_floor)) {
	return;
      }
    }
  }

  p -> last_value = f;
  
  if (NULL != p -> value_widget) {
    gtk_widget_remove_css_class (GTK_WIDGET (p -> value_widget), "value-background-style");
    sprintf (p -> output_value, p -> output_format, f);
    gtk_label_set_text (GTK_LABEL (p -> value_widget), p -> output_value);

    if ((( p -> max > 0) && (f > p -> max))
	|| (( p -> min > 0) && (f < p -> min)) ) {
      gtk_widget_add_css_class (GTK_WIDGET (p -> value_widget), "label-error-style");
      gtk_widget_add_css_class (GTK_WIDGET (p -> label_widget), "label-error-style");
      gtk_widget_add_css_class (GTK_WIDGET (p -> box_widget), "box-error-style");
    } else {
      gtk_widget_remove_css_class (GTK_WIDGET (p -> label_widget), "label-error-style");
      gtk_widget_remove_css_class (GTK_WIDGET (p -> value_widget), "label-error-style");
      gtk_widget_remove_css_class (GTK_WIDGET (p -> box_widget), "box-error-style");
    }
  }
}


extern output_descriptor* output_descriptor_by_name (char*);

static gboolean
idle_task () {
  output_descriptor* p = output_descriptor_by_name ("time");
  if (NULL != p) {
    time_t timer = time (NULL);
    sprintf (p -> output_value, p -> output_format, ctime (&timer));

    /* the string returned by ctime ends with a newline. This fixes that. */
    p -> output_value[strlen (p -> output_value) - 1] = '\0';
    gtk_label_set_text (GTK_LABEL (p -> value_widget), p -> output_value);
  }

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
  float temp;
  struct can_frame frame;
  gsize bytes_read;
  top_level_descriptor* p = top_level_descriptors;
  
  if (G_IO_STATUS_NORMAL != g_io_channel_read_chars (input_channel,
						     (gchar*) &frame, sizeof (struct can_frame),
						     &bytes_read, NULL)) {
    return FALSE;
  }

  if (0 != (call_count++ % 10)) {
    return TRUE;
  }

  while (p < top_level_descriptors + top_level_count) {
    if (p -> frame_descriptor -> id == (frame.can_id & 0x7fffffff)) {
      for (i = 0; i < p -> frame_descriptor -> field_count; i++) {
	/* retrieve the individual data values from the can frame. This handles char and short,
	 * would need to be expanded for 32 or 64 bit data
	 */

	if (NULL == p -> sensor_descriptors[i]) {
	  continue;
	}
	
	if (sizeof (short) == p -> frame_descriptor -> field_sizes[i]) {
	  temp = BytesToShort (frame.data[p -> frame_descriptor -> field_offsets[i]],
					frame.data[p -> frame_descriptor -> field_offsets[i] + 1]);
	} else {
	  temp = frame.data[p -> frame_descriptor -> field_offsets[i]];
	}

	/* apply interpolation if needed */
	temp = linear_interpolate (temp, p -> sensor_descriptors[i]);

	/* MAP sensors typically read 1 ATM at zero boost, this allows those sensors to read 0 at zero boost. */
	temp += p -> sensor_descriptors[i] -> offset;
      
	p -> frame_descriptor -> data[i] = temp;

	if (NULL != p -> output_descriptors[i]) {
	    temp = convert_units (temp, p -> output_descriptors[i] -> units);
	    update_widgets_for_display (p -> output_descriptors[i], temp);
	}
      }

      if (0 != data_logging) {
	log_data (p -> frame_descriptor);
      }

      return TRUE;
    }

    p++;
  }

  return TRUE;
}


static void
init_descriptor_from_builder (output_descriptor* p, GtkBuilder* builder) {
  char scratch[MAX_LABEL_LENGTH];
  GObject* Temp;
      
  if ((NULL == p) || (NULL == builder)) {
    return;
  }

  sprintf (scratch, "label-%d", p -> box_number);
  Temp = gtk_builder_get_object (builder, scratch);
      
  if (NULL == Temp) {
    return;
  }

  p -> label_widget = GTK_WIDGET (Temp);

  gtk_label_set_text (GTK_LABEL (Temp), p -> label);

  sprintf (scratch, "value-%d", p -> box_number);
  Temp = gtk_builder_get_object (builder, scratch);

  if (NULL == Temp) {
    return;
  }

  p -> value_widget = GTK_WIDGET (Temp);

  sprintf (scratch, "box-%d", p -> box_number);
  Temp = gtk_builder_get_object (builder, scratch);
      
  if (NULL == Temp) {
    return;
  }

  p -> box_widget = GTK_WIDGET (Temp);
}   


/*
 * Build the GTK GUI and associate the output widgets with the appropriate data struture so that their values
 * can be updated dynamically.
 */

static void
activate (GtkApplication* app,
          gpointer        user_data) {
  int i;

  GtkBuilder* builder;
  GObject* window;
  GtkCssProvider* provider;
  top_level_descriptor* p = top_level_descriptors;

  builder = gtk_builder_new_from_file (ui_file_name);
  if (NULL == builder) {
    fprintf (stderr, "could not open UI configuration file %s\n", ui_file_name);
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

  gtk_widget_set_visible (GTK_WIDGET (window), TRUE);

  while (p < top_level_descriptors + top_level_count) {
    for (i = 0; i < p -> frame_descriptor -> field_count; i++) {
      if (NULL == p -> output_descriptors[i]) {
	continue;
      }

      gettimeofday (&(p->output_descriptors[i] -> tv), NULL);
      init_descriptor_from_builder (p -> output_descriptors[i], builder);
    }

    p++;
  }

  init_descriptor_from_builder (output_descriptor_by_name ("time"), builder);
  
  /* We do not need the builder any more */
  g_object_unref (builder);
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
      print_config ();
      exit (0);

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

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
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

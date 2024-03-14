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

#include "candinista.h"

#define nBytesToShort(a, b) ((a << 8) | b)
static short
BytesToShort (char a, char b) {
  short x = (a << 8) | b;
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


static void
update_widgets_for_display (output_descriptor* p, float f) {
  if (NULL != p -> value_widget) {
    gtk_widget_remove_css_class (GTK_WIDGET (p -> value_widget), "value-background-style");
    sprintf (p -> value, p -> output_format, f);
    gtk_label_set_text (GTK_LABEL (p -> value_widget), p -> value);

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


/*
 * Called when there is can data ready to be read. Read it from the frame, interpolate and convert if required
 * and update the corresponding display widgets.
 */
static gboolean
can_data_ready (GIOChannel* input_channel, GIOCondition condition, gpointer data)
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
	if (sizeof (short) == p -> frame_descriptor -> field_sizes[i]) {
	  temp = BytesToShort (frame.data[p -> frame_descriptor -> field_offsets[i]],
					frame.data[p -> frame_descriptor -> field_offsets[i] + 1]);
	} else {
	  temp = frame.data[p -> frame_descriptor -> field_offsets[i]];
	}

	/* apply interpolation if needed */
	if (NULL != p -> sensor_descriptors[i]) {
	  temp = linear_interpolate (temp, p -> sensor_descriptors[i]);
	  /* MAP sensors typically read 1 ATM at zero boost, this allows those sensors to read 0 at zero boost. */
	  temp += p -> sensor_descriptors[i] -> offset;
	}

	p -> frame_descriptor -> data[i] = temp;
	temp = convert_units (temp, p->output_descriptors[i] -> units);

	if (NULL != p->output_descriptors[i]) {
	  update_widgets_for_display (p->output_descriptors[i], temp);
	}
      }

      log_data (p -> frame_descriptor);

      return TRUE;
    }

    p++;
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
  int i;

  GtkBuilder* builder;
  GObject* window;
  GtkCssProvider* provider;
  top_level_descriptor* p = top_level_descriptors;

  builder =  gtk_builder_new_from_file (UI_FILE_NAME);
  window = gtk_builder_get_object (builder, "window");
  gtk_window_set_application (GTK_WINDOW (window), app);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_file (provider, g_file_new_for_path (CSS_FILE_NAME));

  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
					      GTK_STYLE_PROVIDER (provider),
					      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  gtk_widget_set_visible (GTK_WIDGET (window), TRUE);

  while (p < top_level_descriptors + top_level_count) {
    for (i = 0; i < p -> frame_descriptor -> field_count; i++) {
      char scratch[MAX_LABEL_LENGTH];
      GObject* Temp;

      sprintf (scratch, "label-%d", p -> output_descriptors[i] -> box_number);
      Temp = gtk_builder_get_object (builder, scratch);
      
      if (NULL == Temp) {
	continue;
      }

      p -> output_descriptors[i] -> label_widget = GTK_WIDGET (Temp);
      
      gtk_label_set_text (GTK_LABEL (Temp),
			  p -> output_descriptors[i] -> label);

      sprintf (scratch, "value-%d", p -> output_descriptors[i] -> box_number);
      Temp = gtk_builder_get_object (builder, scratch);
      
      if (NULL == Temp) {
	continue;
      }
      
      p -> output_descriptors[i] -> value_widget = GTK_WIDGET (Temp);

      sprintf (scratch, "box-%d", p -> output_descriptors[i] -> box_number);
      Temp = gtk_builder_get_object (builder, scratch);
      
      if (NULL == Temp) {
	continue;
      }
      
      p -> output_descriptors[i] -> box_widget = GTK_WIDGET (Temp);
    }

    p++;
  }

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

  read_config_from_json ();

  while (-1 != (option = getopt (argc, argv, "p"))) {
    switch (option) {
    case 'p':
      print_config ();
      exit (0);

    default:
      fprintf (stderr,
	       "Usage: %s options device_name.\n"
	       "Options:\n"
	       "\t -p: print json database\n",
	       argv[0]);
      exit (-1);
    }
  }
    
  get_environment_variables ();
  
  if (NULL == (input_channel = can_setup ())) {
    perror("Error in socket bind");
    return -1;
  }
  
  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_io_add_watch (input_channel, G_IO_IN, can_data_ready, NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}

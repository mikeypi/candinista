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

#include "d3-array.h"
#include "candinista.h"
#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "yaml-printer.h"
#include "gtk-glue.h"
#include "datalogging.h"

Configuration* cfg;
int raw_output = 0;

/*
 * Called when there is can data ready to be read. Read it from the frame, interpolate and convert if required
 * and update the corresponding display widgets.
 */
static gboolean
can_data_ready_task (GIOChannel* input_channel, GIOCondition condition, gpointer data)
{
    (void) condition;
    (void) data;
  
    int i = 0;
    double temp;
    struct can_frame frame;
    gsize bytes_read; 

    if (G_IO_STATUS_NORMAL != g_io_channel_read_chars (input_channel,
                                                       (gchar*) &frame, sizeof (struct can_frame),
                                                       &bytes_read, NULL)) {
        return FALSE;
    }

    /*
     * 1) CAN Data is ready and has been read
     * 2) Find each sensor group with matching CAN ID in turn
     * 3) for each matching sensor group, find the linked panel group and then use
     *      that to find the current panel
     * 4) for each sensor in the sensor group, retrieve the corresponding CAN data
     *      and send to the current panel
     * 4a) have to make sure that ids match so that not all panels have to use
     *      all sensors
     * 4b) have to send multiplexing data (and can_id for some reason)
     */

    sensor_group* sg = cfg -> sensor_groups;

    while (sg < (cfg -> sensor_groups + cfg -> sensor_group_count)) {
        if (((unsigned int) sg -> can_id  & CAN_SFF_MASK)  != (frame.can_id & CAN_SFF_MASK)) {
            i++;
            continue;
        }

        /* reaching here means that a sensor grpup with matching CAN_ID has been
         * located. */

        Panel *p = sg -> linked_panel_group -> current;
        /* All sensor data from a matching sensor group goes to the same panel.
         * Sensors are sorted by row, column so this assumption is safe.
         */

        Sensor* s = sg -> first;

        /* not sure this pointer math works */
        while (s < sg -> last) {
            if (sensor_get_id (s) != panel_get_id (p)) {
                s++;
                /*
                 * The idea here is that a single row column location can have more than one pairing between
                 * sensors and panels. Meaning that some panels may use some sensors, while others use others sensors.
                 */
                continue; 
            }

            /*
             * retrieve the individual data values from the can frame. This handles up to 4 bytes. 
             * would need to be expanded for larger data sizes
             */
            int offset = sensor_get_can_data_offset (s);
            int width = sensor_get_can_data_width(s);

            if (offset < 0 || width < 1 || width > 4 || offset + width > CAN_MAX_DLEN) {
                fprintf (stderr, "Unsupported CAN BUS field width %d or offset %d\n", width, offset);
                continue;
            }
    
            int x = frame.data[offset];
    
            switch (width) {
            case 1:
                break;
            case 2:
                x = (x << 8) | frame.data[offset + 1];
                x = (x << 16);
                x = (x >> 16);
                break;
            case 3:
                x = (x << 8) | frame.data[offset + 1];
                x = (x << 8) | frame.data[offset + 2];
                break;
            case 4:
                x = (x << 8) | frame.data[offset + 1];
                x = (x << 8) | frame.data[offset + 2];
                x = (x << 8) | frame.data[offset + 3];
                break;
            default:
                break;
            }

            temp = x;

            if (0 == raw_output) {
                if (0 != sensor_get_n_values (s)) {
                    /* apply interpolation */
                    temp = linear_interpolate (temp,
                                               sensor_get_x_values (s),
                                               sensor_get_y_values (s),
                                               sensor_get_n_values (s));
                }

                double scale = sensor_get_scale (s);
                if (0 != scale) {
                    temp *= scale;
                }

                temp += sensor_get_offset (s);
            }

            /* not sure this pointer math works */
            panel_set_value (p, temp, s - sg -> first, frame.can_id);
      
            i++;
        }

        /* not sure this pointer math works */
        if ((0 != data_logging) && (0 != (s - sg -> first))) {
            log_data (&frame);
        }
    }

    return TRUE;
}

panel_group *
get_panel_group_for_row_col(int row_index, int column_index) {
    panel_group *pg;
    for (pg = cfg -> panel_groups;
         pg < cfg -> panel_groups + cfg -> panel_group_count; pg++) {
      if ((pg -> current -> row_index == row_index) &&
          (pg -> current -> column_index == column_index)) {
        return (pg);
      }
    }

    return (NULL);
}


/*
 * clicking a panel is supposed to cycle through all of the panels that have the same x, and
 * y but different z coordinates.
 */
static void
on_pressed(GtkGestureClick* gesture,
           int              n_press,
           double           x,
           double           y,
           gpointer         user_data)
{
    (void) gesture;
    (void) n_press;
//    (void) x;
//    (void) y;
  
    typedef struct {
        GtkDrawingArea* drawing_area;
        Panel* cg;
        int timeout_id;
    } cx;

    cx* ctx = (cx*) user_data;

    int column_index = panel_get_column_index (ctx -> cg);
    int row_index = panel_get_row_index (ctx -> cg);

    panel_group* pg = get_panel_group_for_row_col(row_index, column_index);

    unsigned long aa = (unsigned long)pg -> first;
    unsigned long bb = (unsigned long)pg -> last;
    unsigned long cc = (unsigned long)ctx -> cg - aa;
    unsigned long dd = bb - aa;
    unsigned long ee = (cc + 1) % dd;
    
    ptrdiff_t delta = (ctx -> cg - pg -> first) & 0xff;
    ptrdiff_t span = pg -> last - pg -> first;
    unsigned long active_layer = (delta + 1) % span;
    pg -> current = pg -> first + active_layer;

//    fprintf (stderr, "sizeof panel %ld\n", sizeof (Panel));
//    fprintf (stderr, "first = %p, last = %p\n", (void*) pg -> first, (void*) pg -> last);
//    fprintf (stderr, "delta %td, span %td, new active_layer %td\n", delta, span, active_layer);
//    fprintf(stderr, "pg = %p, first = %p, last = %p\n", (void *)pg,
//            (void *)pg->first, (void *)pg->last);
    unsigned long a = (unsigned long)pg -> first;
    unsigned long b = (unsigned long)pg -> last;
    
    fprintf(stderr, "first = %ld last = %ld delta = %td\n",
            a, b, b - a);
#if 0
    Panel* p = NULL;
  
    for (int i = 1; i < cfg -> z_dimension; i++) {
        int j = (active_z + i) % cfg -> z_dimension;
        if (NULL != (p = cfg_get_panel (cfg, column_index, row_index, j))) {
            cfg_set_active_z (cfg, column_index, row_index, j);
            break;
        }
    }
#endif

    if (NULL != pg -> current) {
        ctx -> cg = pg -> current;
        gtk_drawing_area_set_draw_func(
            ctx -> drawing_area, gtk_draw_gauge_panel_cb, pg -> current, NULL);
        
        int timeout = panel_get_timeout (pg -> current);
        g_source_remove (ctx -> timeout_id);
        ctx -> timeout_id =
            g_timeout_add((0 == timeout) ? DEFAULT_TIMEOUT : timeout,
                          gtk_update_gauge_panel_value, ctx);
    }
}


static void
on_drawing_area_destroy (GtkWidget *widget, gpointer user_data)
{
    (void) widget;
  
    struct {
        GtkDrawingArea* drawing_area;
        Panel* cg;
        int timeout_id;
    }* ctx = user_data;

    if (0 != ctx -> timeout_id) {
        g_source_remove (ctx -> timeout_id);
    }

    g_free (ctx);
}


/*
 * Build the GTK GUI and associate the output widgets with the appropriate data struture so that their values
 * can be updated dynamically.
 */
static void
activate (GtkApplication* app,
          gpointer        user_data) {

    (void) user_data;
    GtkWidget* window;
    Panel* p;

    window = gtk_window_new ();
    g_return_if_fail (GTK_IS_WIDGET (window));

    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 600);
    gtk_window_set_application (GTK_WINDOW (window), app);

    if (0 == remote_display) {
        gtk_window_fullscreen (GTK_WINDOW(window));
    }

    GtkGrid *grid = GTK_GRID(gtk_grid_new());
    g_return_if_fail (GTK_IS_WIDGET (grid));

    gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET(grid));
    gtk_grid_set_column_homogeneous (grid, TRUE);


    panel_group *pg;

    for (pg = cfg -> panel_groups;
         pg < cfg -> panel_groups + cfg -> panel_group_count; pg++) {
        GtkDrawingArea* drawing_area = GTK_DRAWING_AREA(gtk_drawing_area_new());

        gtk_widget_set_hexpand (GTK_WIDGET(drawing_area), TRUE);
        gtk_widget_set_vexpand (GTK_WIDGET(drawing_area), TRUE);

        p = pg -> current;
            
        assert (NULL != p);
	
        struct {
            GtkDrawingArea* drawing_area;
            Panel* cg;
            int timeout_id;
        }* ctx = g_new0 (typeof (*ctx), 1);

        guint timeout = panel_get_timeout (p);

        ctx -> drawing_area = drawing_area;
        ctx -> cg = p;
        ctx -> timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT,
                                                (timeout == 0) ? 600 : timeout,
                                                gtk_update_gauge_panel_value,
                                                ctx,
                                                NULL);

        g_signal_connect (drawing_area, "destroy",
                          G_CALLBACK (on_drawing_area_destroy), ctx);

        gtk_drawing_area_set_draw_func (drawing_area, gtk_draw_gauge_panel_cb, p, NULL);
      
        GtkGesture* click = gtk_gesture_click_new();
        g_signal_connect (click, "pressed",
                          G_CALLBACK (on_pressed), ctx);

        gtk_widget_add_controller (GTK_WIDGET (drawing_area), GTK_EVENT_CONTROLLER (click));
        gtk_grid_attach(grid, GTK_WIDGET(drawing_area), p -> column_index, p -> row_index, 1, 1);
    }

    gtk_window_present(GTK_WINDOW(window));
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

    can_socket_name = "vcan0";
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
    int option;

    get_environment_variables ();

    if (NULL == (cfg = configuration_load_yaml (config_file_name))) {
        fprintf (stderr, "unable to open config file %s\n", config_file_name);
        exit (-1);
    }
  
    cfg_build_tables (cfg);

    while (-1 != (option = getopt (argc, argv, "dpr"))) {
        switch (option) {
        case 'd':
            data_logging = 1;
            break;
      
        case 'p':
            configuration_print (stdout, cfg);
            exit (0);
            break;

        case 'r':
            raw_output = 1;
            break;

        default:
            fprintf (stderr,
                     "Usage: %s options device_name.\n"
                     "Options:\n"
                     "\t -d: enable datalogging\n"
                     "\t -p: print yaml database\n"
                     "\t -r: print raw sensor values\n", argv[0]);

            exit (-1);
        }
    }

    if (NULL == (input_channel = can_setup ())) {
        perror ("Error in socket bind");
        return -1;
    }

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    g_io_add_watch (input_channel, G_IO_IN, can_data_ready_task, NULL);

    /* not sure why this is required, but g_application_run will throw an error if it is called with
     * additional flags in argv (e.g., -n).
     */
    argv[1] = NULL;

    argc = 1;
    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref (app);
    cfg_free (cfg);
  
    return status;
}
       

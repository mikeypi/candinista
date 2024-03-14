#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

FILE* fp = NULL;

int log_file_open_time = 0;


static void
log_file_open (int open_time) {
  char  filepath[256];

  if (NULL != fp) {
    fclose (fp);
    fp = NULL;
  }
  
  sprintf (filepath, "%s/%s%d.csv", log_file_directory_name, "Candata", open_time);
  fp = fopen (filepath, "w");
}


void
log_data (frame_descriptor *p) {
  int i;
  struct timeval tv;

  gettimeofday (&tv, NULL);

  if ((tv.tv_sec - log_file_open_time) > max_log_file_time) {
    log_file_open (tv.tv_sec);
    log_file_open_time = tv.tv_sec;
  }

  if (NULL == fp) {
    return;
  }
  
  fprintf (fp, "%d:%d,%X,", tv.tv_sec, tv.tv_usec, p -> id);
  
  for (i = 0; i < p -> field_count; i++) {
    fprintf (fp, "%.4f", p -> data[i]);
    if (i != (p -> field_count - 1)) {
      fprintf (fp, ",");
    }
  }

  fprintf (fp, "\n");
}

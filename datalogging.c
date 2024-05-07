
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
#include <sys/time.h>

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

FILE* fp = NULL;

int log_file_open_time = 0;
int data_logging = 0;

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

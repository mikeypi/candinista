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
#include <limits.h>
#include <sys/time.h>
#include <sys/param.h>
#include <linux/can.h>

#include "candinista.h"

int data_logging = 0;


static FILE*
log_file_open (FILE* fp) {
  static int file_number = 0;
  char  filepath[PATH_MAX];

  if (NULL != fp) {
    fclose (fp);
    fp = NULL;
  }
  
  sprintf (filepath, "%s/%s.%d.%d.csv", log_file_directory_name, "Candata", getpid (), file_number++);
  return (fopen (filepath, "w"));
}


static void print_full_record (struct can_frame* p1, struct can_frame* p2, struct timeval* file_start_time, struct timeval* current_time, FILE* fp) {
  int i;

  fprintf (fp, "%d:%d,%X,", current_time -> tv_sec - file_start_time -> tv_sec, current_time -> tv_usec, p1 -> can_id);
  for (i = 0; i < CAN_MAX_DLEN; i++) {
    fprintf (fp, "%d", p1 -> data[i]);
    if (i != (CAN_MAX_DLEN - 1)) {
      fprintf (fp, ",");
    }
  }

  fprintf (fp, "\n");
}


static void print_sparse_record (struct can_frame* p1, struct can_frame* p2, struct timeval* file_start_time, struct timeval* current_time, FILE* fp) {
  int i;

  fprintf (fp, "%d:%d,,", current_time -> tv_sec - file_start_time -> tv_sec, current_time -> tv_usec);
  for (i = 0; i < CAN_MAX_DLEN; i++) {
    if (p1 -> data[i] != p2 -> data[i]) {
      fprintf (fp, "%d", p1 -> data[i]);
    }
    if (i != (CAN_MAX_DLEN - 1)) {
      fprintf (fp, ",");
    }
  }

  fprintf (fp, "\n");
}


void
log_data (struct can_frame* p1) {
  int i;
  static struct timeval file_start_time;
  static struct timeval current_time;
  static struct timeval temp;
  static int call_count = 1;
  static struct can_frame p2;
  static FILE* fp = NULL;

  /*
   * This should happens only the first time that this function is called. Because this is the first call,
   * this section adds the configuration (as obtained from the JSON config file) as part of the file header.
   */
  if (NULL == fp) {
    if (NULL == (fp = log_file_open (fp))) {
      fprintf (stderr, "open for log file in %s failed in %s at %d\n",
	       __FILE__, __LINE__,log_file_directory_name);
      return;
    }

    //    print_config (fp);
    gettimeofday (&file_start_time, NULL);
    fprintf (fp, "File Start at: %d:%d\n\n", file_start_time.tv_sec, file_start_time.tv_usec);
    current_time = file_start_time;
    print_full_record (p1, &p2, &file_start_time, &current_time, fp);
    p2.can_id = p1 -> can_id;
    memcpy (p2.data, p1 -> data, sizeof (p1 -> data));

    return;
  }

  /*
   * This should happens every nth time function is called. This does the same stuff as the preceding section, but does
   * not include the JSON configuration information.
   */
  if (0 == (call_count++ % 10000)) {
    if (NULL == (fp = log_file_open (fp))) {
      fprintf (stderr, "open for log file in %s failed in %s at %d\n",
	       __FILE__, __LINE__,log_file_directory_name);
      return;
    }

    gettimeofday (&file_start_time, NULL);
    fprintf (fp, "File Start at: %d:%d\n\n", file_start_time.tv_sec, file_start_time.tv_usec);
    current_time = file_start_time;
    print_full_record (p1, &p2, &file_start_time, &current_time, fp);
    p2.can_id = p1 -> can_id;
    memcpy (p2.data, p1 -> data, sizeof (p1 -> data));

    return;
  }
  
  /* arbitrary test to reduce logging resolution (and file size) */
  gettimeofday (&temp, NULL);
  if ((temp.tv_sec == current_time.tv_sec) 
      && (temp.tv_usec - current_time.tv_usec) < 40) {
    return;
  }
  current_time = temp;

  /* if the current CAN id is not the same as the last CAN id, print the entire record and return */
  if (p1 -> can_id != p2.can_id) {
    print_full_record (p1, &p2, &file_start_time, &current_time, fp);
    p2.can_id = p1 -> can_id;
    memcpy (p2.data, p1 -> data, sizeof (p1 -> data));
    return;
  }   

  /* suppress logging of individual bytes if data has not changed */
  print_sparse_record (p1, &p2, &file_start_time, &current_time, fp);
  p2.can_id = p1 -> can_id;
  memcpy (p2.data, p1 -> data, sizeof (p1 -> data));
}

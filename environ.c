
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

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

char* log_file_directory_name = LOG_FILE_DIRECTORY_NAME;
int max_log_file_time = MAX_LOG_FILE_TIME;
char* can_socket_name = CAN_SOCKET_NAME;
char* ui_file_name = UI_FILE_NAME;
char* css_file_name = CSS_FILE_NAME;


void
get_environment_variables () {
  char* temp;

  temp = getenv ("CANDINISTA.LOG_FILE_DIRECTORY_NAME");
  if (NULL != temp) {
    log_file_directory_name = temp;
  }

  temp = getenv ("CANDINISTA.MAX_LOG_FILE_TIME");
  if (NULL != temp) {
    sscanf (temp, "%d", max_log_file_time);
  }

  temp = getenv ("CANDINISTA.CAN_SOCKET_NAME");
  if (NULL != temp) {
    can_socket_name = temp;
  }

  temp = getenv ("CANDINISTA.UI_FILE_NAME");
  if (NULL != temp) {
    ui_file_name = temp;
  }

  temp = getenv ("CANDINISTA.CSS_FILE_NAME");
  if (NULL != temp) {
    css_file_name = temp;
  }
}

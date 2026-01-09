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
#include <linux/can.h>

#include "candinista.h"

char* log_file_directory_name = LOG_FILE_DIRECTORY_NAME;
char* can_socket_name = CAN_SOCKET_NAME;
char* ui_file_name = UI_FILE_NAME;
char* css_file_name = CSS_FILE_NAME;
char* config_file_name = CONFIG_FILE_NAME;
char* candinista_working_directory = NULL;
int remote_display = 0;

void
get_environment_variables () {
  char* t1;

  t1 = getenv ("CANDINISTA_LOG_FILE_DIRECTORY_NAME");
  if (NULL != t1) {
    log_file_directory_name = t1;
  }

  t1 = getenv ("CANDINISTA_CAN_SOCKET_NAME");
  if (NULL != t1) {
    can_socket_name = t1;
  }

  t1 = getenv ("CANDINISTA_UI_FILE_NAME");
  if (NULL != t1) {
    ui_file_name = t1;
  }

  t1 = getenv ("CANDINISTA_CSS_FILE_NAME");
  if (NULL != t1) {
    css_file_name = t1;
  }

  t1 = getenv ("CANDINISTA_WORKING_DIRECTORY");
  if (NULL != t1) {
    if (0 != chdir (t1)) {
      char temp[PATH_MAX];
      fprintf (stderr, "Unable to change working directory from: %s to: %s\n", getcwd (temp, sizeof(temp)), t1);
    }
  }

  t1 = getenv ("CANDINISTA_CONFIG_FILE_NAME");
  if (NULL != t1) {
    config_file_name = t1;
  }

  t1 = getenv ("DISPLAY");
  if (NULL != t1) {
    if (0 != strcmp (t1, ":0")) {
      remote_display = 1;
    }
  }
}

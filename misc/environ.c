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
char* config_file_name = CONFIG_FILE_NAME;
char* working_directory = NULL;
int remote_display = 0;

void
get_environment_variables () {
  char* t1;

  t1 = getenv ("CANDINISTA_WORKING_DIRECTORY");
  if (NULL != t1) {
    working_directory = strdup (t1);
  } else {
    working_directory = "./";
  }

  t1 = getenv (LOG_FILE_DIRECTORY_NAME);
  if (NULL != t1) {
    char* t2 = (char*) malloc (strlen (t1) + strlen (working_directory) + 2);
    sprintf (t2, "%s/%s", working_directory, t1);
    log_file_directory_name = t2;
  } else {
    char* t2 = (char*) malloc (strlen (LOG_FILE_DIRECTORY_NAME) + strlen (working_directory) + 2);
    sprintf (t2, "%s/%s", working_directory, LOG_FILE_DIRECTORY_NAME);
    log_file_directory_name = t2;
  }

  t1 = getenv (CONFIG_FILE_NAME);
  if (NULL != t1) {
    char* t2 = (char*) malloc (strlen (t1) + strlen (working_directory) + 2);
    sprintf (t2, "%s/%s", working_directory, t1);
    config_file_name = t2;
  } else {
    char* t2 = (char*) malloc (strlen (CONFIG_FILE_NAME) + strlen (working_directory) + 2);
    sprintf (t2, "%s/%s", working_directory, CONFIG_FILE_NAME);
    config_file_name = t2;
  }

  t1 = getenv ("CANDINISTA_CAN_SOCKET_NAME");
  if (NULL != t1) {
    can_socket_name = t1;
  }

  t1 = getenv ("DISPLAY");
  if (NULL != t1) {
    if (0 != strcmp (t1, ":0")) {
      remote_display = 1;
    }
  }
}

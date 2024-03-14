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

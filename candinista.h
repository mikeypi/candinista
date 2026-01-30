#ifndef CANDINISTA_H
#define CANDINISTA_H

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

#define MAX_CAN_FIELDS 8
#define MAX_LABEL_LENGTH 40

typedef enum {CAIRO_GAUGE_PANEL, CAIRO_INFO_PANEL, CAIRO_BARGRAPH_PANEL} output_type;

/* data logging stuff */

extern int data_logging;

/* interpolation stuff */
extern void interpolation_array_sort (double* x_values, double* y_values, int number_of_interpolation_points);
extern double linear_interpolate (double knownx, double* x_values, double* y_values, int number_of_interpolation_points);
/* Initialization stuff */

#define LOG_FILE_DIRECTORY_NAME "datalogs"
extern char* log_file_directory_name;

#define CAN_SOCKET_NAME "can0"
extern char* can_socket_name;

#define CONFIG_FILE_NAME "config.yaml"
extern char* config_file_name;

extern int remote_display;

extern void get_environment_variables ();
#endif

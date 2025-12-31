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

typedef enum {FAHRENHEIT, CELSIUS, BAR, PSI, NONE} unit_type;

/*
 * GUI widgets to display values along with an associated label (temp, rpm etc.). The descriptors are
 * defined in JSON and the widgets are additionally specified as XML, and the builder_names in this
 * structure are for finding those widgets and storing pointers to them in this structure.
 */
typedef struct {
  char* name;
  int id;

  double value;
  
  double min;
  double max;
  double low_warn;
  double high_warn;
  char* label;
  char* legend;

  unit_type units;  
  double offset;

  int row;
  int column;

  void* output;
  struct timeval tv;
  int update_interval;
} output_descriptor;


/* values retrived from a can frame generally have to be converted into corresponding sensor values.
 * For example, temperature vales in a can frame are often measured in ohms and have to be converted
 * to degrees. Sensor manufacturers generally provide a series of interpolation points (x and corresponding
 * y values). This structure holds those values for a specific can frame value. Also includes an enum to
 * specify if units need to be changed from C to F, etc. "offset" is a value that is added to any
 * reading post interpolation but pre-conversion. It's specifically to support MAP sensors
 * which generally read 1 ATM at zero boost. 
 */
typedef struct {
  char* name;
  int number_of_interpolation_points;
  float* x_values;
  float* y_values;

  int can_id;
  int can_data_offset;
  int can_data_width;
  output_descriptor* output_descriptor;
} sensor_descriptor;


extern sensor_descriptor* sensor_descriptors;
extern output_descriptor* output_descriptors;
extern int sensor_count;
extern int output_count;

/* data logging stuff */

extern int data_logging;
extern void log_data (struct can_frame*);

/* interpolation stuff */
extern void interpolation_array_sort (sensor_descriptor*);
extern float linear_interpolate (float, sensor_descriptor*);

/* Initialization stuff */

#define LOG_FILE_DIRECTORY_NAME "datalogs"
extern char* log_file_directory_name;

#define CAN_SOCKET_NAME "can0"
extern char* can_socket_name;

#define UI_FILE_NAME "candinista.ui"
extern char* ui_file_name;

#define CSS_FILE_NAME "styles.css"
extern char* css_file_name;

#define CONFIG_FILE_NAME "config.json"
extern char* config_file_name;

extern int remote_display;

extern void get_environment_variables ();

extern void read_config_from_json (void);
extern void print_config (FILE*);

#define DISPALY_UPDATE_INTERVAL 2


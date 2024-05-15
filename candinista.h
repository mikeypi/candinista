
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

/* each can frame can include up to 8 bytes of data. That can be 8 individual 8-bit values,
 * or 4 16-bit values or any other combination of 8 to 64-bit data. For a specific can frame,
 * this structure describes the size and location of a particular value.
 */
typedef struct {
  int id; /* The can frame id */
  int field_count; /* number of values (up to 8) in a specific can frame */
  int field_offsets[MAX_CAN_FIELDS];
  int field_sizes[MAX_CAN_FIELDS];
  float data[MAX_CAN_FIELDS]; /* place for the values from the frame to be stored */
  char* name;
} frame_descriptor;

/* values retrived from a can frame generally have to be converted into corresponding sensor values.
 * For example, temperature vales in a can frame are often measured in ohms and have to be converted
 * to degrees. Sensor manufacturers generally provide a series of interpolation points (x and corresponding
 * y values). This structure holds those values for a specific can frame value. Also includes an enum to
 * specify if units need to be changed from C to F, etc. "offset" is a value that is added to any
 * reading post interpolation but pre-conversion. It's specifically to support MAP sensors
 * which generally read 1 ATM at zero boost. 
 */
typedef struct {
  int number_of_interpolation_points;
  float* x_values;
  float* y_values;
  float offset;  
  char* name;
} sensor_descriptor;

/*
 * GUI widgets to display values along with an associated label (temp, rpm etc.). The descriptors are
 * defined in JSON and the widgets are additionally specified as XML, and the builder_names in this
 * structure are for finding those widgets and storing pointers to them in this structure.
 */
typedef struct {
  char* label;
  int box_number;
  int min;
  int max;
  float last_value;
  char output_value[MAX_LABEL_LENGTH];
  unit_type units;
  char* output_format;
  GtkWidget* label_widget;
  GtkWidget* value_widget;
  GtkWidget* box_widget;
  char* name;
  struct timeval tv;
  int update_interval;
  int update_floor;
} output_descriptor;

/* structure to assocaite a particular can frame with its sensors and output structures. */
typedef struct {
  frame_descriptor* frame_descriptor;
  output_descriptor* output_descriptors[MAX_CAN_FIELDS];
  sensor_descriptor* sensor_descriptors[MAX_CAN_FIELDS];
  int output_descriptor_count;
  int sensor_descriptor_count;
} top_level_descriptor;

extern top_level_descriptor* top_level_descriptors;
extern int top_level_count;

/* data logging stuff */

extern int data_logging;
extern void log_data (frame_descriptor*);

/* interpolation stuff */
extern void interpolation_array_sort (sensor_descriptor*);
extern float linear_interpolate (float, sensor_descriptor*);

/* Initialization stuff */

#define LOG_FILE_DIRECTORY_NAME "datalogs"
extern char* log_file_directory_name;

#define MAX_LOG_FILE_TIME 10*60
extern int max_log_file_time;

#define CAN_SOCKET_NAME "can0"
extern char* can_socket_name;

#define UI_FILE_NAME "candinista.ui"
extern char* ui_file_name;

#define CSS_FILE_NAME "styles.css"
extern char* css_file_name;

extern void get_environment_variables ();

extern void read_config_from_json (void);
extern void print_config (void);

#define DISPALY_UPDATE_INTERVAL 2


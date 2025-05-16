
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

#include <linux/can.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "candinista.h"

static int
comp1 (const void * elem1, const void * elem2) 
{
    float f = *((float*) elem1);
    float s = *((float*) elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}


static int
comp2 (const void * elem1, const void * elem2) 
{
    float f = *((float*) elem1);
    float s = *((float*) elem2);
    if (f < s) return  1;
    if (f > s) return -1;
    return 0;
}

/*
 * In general, sensor specification provide reading/value pairs so that the values are in
 * ascending order. This routine allows the sensor_descriptor definitions to be in
 * ascending or descending orders.
 */
void
interpolation_array_sort (sensor_descriptor* sensor) {
  int i;

  if (1 > sensor -> number_of_interpolation_points) {
    return;
  }

  if (sensor -> x_values[0] < sensor -> x_values [1]) {
    return;
  }
  
  qsort (sensor -> x_values, sensor -> number_of_interpolation_points, sizeof(float), comp1);

  if (sensor -> y_values[0] < sensor -> y_values [1]) {
      qsort (sensor -> y_values, sensor -> number_of_interpolation_points, sizeof(float), comp2);
  } else {
    qsort (sensor -> y_values, sensor -> number_of_interpolation_points, sizeof(float), comp1);
  }
}


/*
 * This is harder than it looks. It's possible that the value being interpolated is either equal to
 * an end point of the range or lies entirely outside of the range. A common way to handle that is
 * make up additional data points beyond the range, but that really doesn't solve the issue for
 * points even further outside of the range. For that reason, and because of the way that gauges
 * traditionally work, its better to return the values associated with end points for all points
 * at or beyond the range. This function assumes that the range values are sorted in increasing
 * order.
 */
 
float
linear_interpolate (float knownx, sensor_descriptor* sensor)
{
  int i = 0;
  float mu;
  float y1;
  float y2;
  float result;

  if (knownx <= sensor -> x_values[0]) {
    return (sensor -> y_values[0]);
  }
  
  if (knownx >= sensor -> x_values[sensor -> number_of_interpolation_points - 1]) {
    return (sensor -> y_values[sensor -> number_of_interpolation_points - 1]);
  }

  while (i < sensor -> number_of_interpolation_points) { 
    if (knownx == sensor -> x_values[i]) {
      return (sensor -> y_values[i]);
    }

    if (knownx < sensor -> x_values[i]) {
      ++i;
      continue;
    }

    if (knownx < sensor -> x_values[i + 1]) {
      break;
    }

    ++i;
  }

  mu = (knownx - sensor -> x_values[i]) / (sensor -> x_values[i + 1] - sensor -> x_values[i]);
  y1 = sensor -> y_values[i];
  y2 = sensor -> y_values[i + 1];
  result = y1 * (1 - mu) + y2 * mu;

#ifdef DEBUG_INTERPOLATE
  fprintf (stderr, "mu = %f y1 = %f y = %f, return = %f knownx = %f\n",
	   mu,
	   sensor->x_values[i],
	   sensor->x_values[i+1],
	   result,
	   knownx);
#endif

  return (result);
}


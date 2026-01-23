
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
#include <stdio.h>
#include <stdlib.h>

#include "candinista.h"

static int
comp1 (const void * elem1, const void * elem2) 
{
    double f = *((double*) elem1);
    double s = *((double*) elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}


static int
comp2 (const void * elem1, const void * elem2) 
{
    double f = *((double*) elem1);
    double s = *((double*) elem2);
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
interpolation_array_sort (double* x_values, double* y_values, int number_of_interpolation_points) {

  if (1 > number_of_interpolation_points) {
    return;
  }

  if (x_values[0] < x_values [1]) {
    return;
  }
  
  qsort (x_values, number_of_interpolation_points, sizeof(double), comp1);

  if (y_values[0] < y_values [1]) {
      qsort (y_values, number_of_interpolation_points, sizeof(double), comp2);
  } else {
    qsort (y_values, number_of_interpolation_points, sizeof(double), comp1);
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
 
double
linear_interpolate (double knownx, double* x_values, double* y_values, int number_of_interpolation_points)
{
  int i = 0;
  double mu;
  double y1;
  double y2;
  double result;

  if (knownx <= x_values[0]) {
    return (y_values[0]);
  }
  
  if (knownx >= x_values[number_of_interpolation_points - 1]) {
    return (y_values[number_of_interpolation_points - 1]);
  }

  while (i < number_of_interpolation_points) { 
    if (knownx == x_values[i]) {
      return (y_values[i]);
    }

    if (knownx < x_values[i]) {
      ++i;
      continue;
    }

    if (knownx < x_values[i + 1]) {
      break;
    }

    ++i;
  }

  mu = (knownx - x_values[i]) / (x_values[i + 1] - x_values[i]);
  y1 = y_values[i];
  y2 = y_values[i + 1];
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


#ifdef DEBUG
//double x_values[] = {5000, 10000, 15000, 20000, 25000, 27500, 80000};
//double y_values[] = {2500, 5000, 7500, 10000, 12500, 13750, 40000};

double x_values[] = {0, 27500};
double y_values[] = {0, 0.7*27500};

int
main (int argc, char** argv) {
  int i;
  int n_values = sizeof (x_values)/sizeof (double);
  interpolation_array_sort (x_values, y_values, n_values);
  for (i = 0; i < n_values; i++) {
    double d = linear_interpolate (x_values[i], x_values, y_values, n_values);
    fprintf (stderr, "interpolated %f, expected %f, got %f\n", x_values[i], y_values[i], d);
  }

  fprintf (stderr, "\n");
  
  for (i = 0; i < n_values - 1; i++) {
    double d = linear_interpolate ((x_values[i] + x_values[i+1])/2.0,
				   x_values, y_values, n_values);

    fprintf (stderr, "interpolated %f, expected %f, got %f\n",
	     (x_values[i] + x_values[i+1])/2.0,
	     (y_values[i] + y_values[i+1])/2.0,
	     d);
  }
}
#endif

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "candinista.h"

#define MAX_SENSOR_VALUE 2000

/*
 *
 * Sensor descriptors.
 *
 */
  
/* Interpolation data for bosch pressure sensor part PST-F1. */
sensor_descriptor bosch_pst_f1_pressure = {
  2,
  {
    0.5, 4.5
  },
  {
    0, 1000
  }
};

/* Interpolation data for bosch temperature sensor part PST-F1. */
sensor_descriptor bosch_pst_ft_temp = {
  19,
  {
    71.9, 89.9, 113.3, 144.5, 186.6, 244, 323.4, 434.9, 594, 824, 1167, 1683, 2480, 3740, 5784, 9195, 15067, 25524, 44864
  },
  {
    140, 130, 120, 110, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0, -10, -20, -30, -40
  }
};
  
/* Interpolation data for bosch MAP sensor part 261230119. */
sensor_descriptor bosch_261230119 = {
  6,
  {
    0.096428571, 0.4, 1.614285714, 3.132142857, 4.65, 6.167857143
  },
  {
    0, 20, 100, 200, 300, 400
  }
};

/* Interpolation data for bosch fluid temperature sensor part ntc m12. */
sensor_descriptor bosch_ntc_m12 = {
  18,
  {
    89, 113, 144, 187, 243, 323, 436, 596, 834, 1175, 1707, 2500, 3792, 5896, 9397, 15462, 26114, 45313
  },
  {
    130, 120, 110, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0, -10, -20, -30, -40
  }
};

/* Interpolation data for bosch air temperature sensor part ntc m12-L. */
sensor_descriptor bosch_ntc_m12_l = {
  19,
  {
    71, 89, 113, 144, 187, 243, 323, 436, 596, 834, 1175, 1707, 2500, 3792, 5896, 9397, 15462, 26114, 45313
  },
  {
    140, 130, 120, 110, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0, -10, -20, -30, -40
  }
};

/* Interpolation data for delphi water temperature sensor part #15326386. */
sensor_descriptor delphi_15326386 = {
  39,
  {
    48, 53, 60, 68, 78, 88, 101, 116, 133, 154, 178, 207, 242, 283, 334, 395, 469,
    559, 671, 809, 980, 1195, 1465, 1806, 2240, 2795, 3511, 4441, 5658, 7263, 9399,
    12261, 16120, 21371, 28582, 38583, 52594, 72437, 100865
  },
  {
    150, 145, 140, 135, 130, 125, 120, 115, 110, 105, 100, 95, 90, 85, 80, 75, 70,
    65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0, -5, -10, -15, -20, -25, -30,
    -35, -40
  }
};

/* Interpolation data for delphi fluid temperature sensor part ntc m12. */
sensor_descriptor delphi_25036751 = {
  39,
  {
    46.7, 52.6, 59.4, 67.3, 76.4, 87, 99.4, 113.9, 131, 151.3, 175.3, 203.9, 238.1, 279, 329, 389,
    462, 551, 660, 796, 965, 1177, 1443, 1778, 2205, 2752, 3457, 4373, 5572, 7153, 9256, 12073,
    15873, 21044, 28146, 37994, 51791, 71332, 99326
  },
  {
    150, 145, 140, 135, 130, 125, 120, 115, 110, 105, 100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50,
    45, 40, 35, 30, 25, 20, 15, 10, 5, 0, -5, -10, -15, -20, -25, -30, -35, -40
  }
};

sensor_descriptor XXX = {
  7,
  {
    7000, 6000, 5000, 4000, 3000, 2000, 1000
  },
  {
    33, 46.7, 48.6, 59.4, 67.3, 76.4, 87
  }
};

sensor_descriptor YYY = {
  7,
  {
    1000, 2000, 3000, 4000, 5000, 6000, 7000
  },
  {
    87, 76.4, 67.3, 59.4, 48.6, 46.7, 33
  }
};


/*
 *
 * Output descriptors.
 *
 */
  
output_descriptor box0 = {
  "Water\nTemp", 0, 0, 190
};

output_descriptor box1 = {
  "Intake\nTemp", 1, 0, 190
};

output_descriptor box2 = {
  "Oil\nTemp", 2, -MAX_SENSOR_VALUE, MAX_SENSOR_VALUE
};

output_descriptor box3 = {
  "Oil\nPressure", 3, -MAX_SENSOR_VALUE, MAX_SENSOR_VALUE
};

output_descriptor box4 = {
  "MAP", 4, -MAX_SENSOR_VALUE, MAX_SENSOR_VALUE
};

output_descriptor box5 = {
  "RPM", 5, -MAX_SENSOR_VALUE, MAX_SENSOR_VALUE
};

output_descriptor box6 = {
  "VBAT", 6, -MAX_SENSOR_VALUE, MAX_SENSOR_VALUE
};
  

/*
 *
 * Frame descriptors.
 *
 */
  
/* AEM P/N 30-2226 6 CHANNEL CAN SENSOR MODULE produces three different CAN frames. This one, identified by CAN header
 * 0xb600 includes four sixteen bit values, two temperature readings and two pressure readings. This descriptor
 * descibes that frame, attached to two delphi 15326386 temperature sensors.
 */
frame_descriptor b6000frame = {
  0xb600,
  B6000_FIELD_COUNT,
  {
    /* field offsets */
    0, 2, 4, 6
  },
  {
    /* field sizess */
    sizeof (short), sizeof (short), sizeof (short), sizeof (short)
  }
};
  
frame_descriptor b6001frame = {
  0xb601,
  B6001_FIELD_COUNT,
  {
    /* field offsets */
    0, 6, 7
  },
  {
    /* field sizess */
    sizeof (short), sizeof (char), sizeof (char)
  }
};

top_level_descriptor _top_level_descriptors[] = {
  {
    &b6000frame,
    {
      &box0,
      &box1,
      &box2,
      &box3
    },
    {
      &delphi_15326386,
      &delphi_15326386,
      &XXX,
      &YYY
    }
  },
  {
    &b6001frame,
    {
      &box4,
      &box5,
      &box6,
    },
    {
      &XXX,
      &XXX,
      &XXX,
      &XXX
    }
  }
};

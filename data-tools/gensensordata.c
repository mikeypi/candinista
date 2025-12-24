#include <stdio.h>
#include <math.h>

extern double sine (double);


int main (int argc, char* argv[]) {
  {
    double d = 0;
    short int i;
    union {
      short int i;
      char bytes[2];
    } X;
  
    X.i = 0;
    
    while (d < 240) {
      //      X.i = 65536.0 * (1.0 + sin (d));
      d += 0.04;
      X.i +=1;
      printf ("0:740770,0x8000b000,");
      printf ("%d,%d,", X.bytes[1], X.bytes[0]);
            printf ("%d,%d,", X.bytes[1], X.bytes[0]);
	          printf ("%d,%d,", X.bytes[1], X.bytes[0]);
		        printf ("%d,%d,", X.bytes[1], X.bytes[0]);

      printf ("\n");

      printf ("0:740770,0x8000b003,");
      printf ("%d,%d,", X.bytes[1], X.bytes[0]);
            printf ("%d,%d,", X.bytes[1], X.bytes[0]);
	          printf ("%d,%d,", X.bytes[1], X.bytes[0]);
		        printf ("%d,%d,", X.bytes[1], X.bytes[0]);

      printf ("\n");
    }
  }
}

#ifdef aljdakjs

"CAN Id": "0xb000",
  "CAN Data Offset": 0,
  "CAN Data Width": 2,
  "CAN Data Offset": 2,
  "CAN Data Width": 2,
  "CAN Data Offset": 4,
  "CAN Data Width": 2,



  "CAN Id": "0xb003",
  "CAN Data Offset": 0,
  "CAN Data Width": 2,
  "CAN Data Offset": 2,
  "CAN Data Width": 2,
  "CAN Data Offset": 4,
  "CAN Data Width": 2,
  "CAN Data Offset": 6,
  "CAN Data Width": 2
  }

	      
#endif


#ifndef _IEEE754_H

#define _IEEE754_H 1

union ieee754_float
  {
    float f;

    /* This is the IEEE 754 single-precision format.  */
    struct
      {

    unsigned long int mantissa:23;
    unsigned long int exponent:8;
    unsigned long int negative:1;
      } ieee;

    /* This format makes it easier to see if a NaN is a signalling NaN.  */
    struct
      {

//#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned long int mantissa:22;
    unsigned long int quiet_nan:1;
    unsigned long int exponent:8;
    unsigned long int negative:1;
//#endif              /* Little endian.  */
      } ieee_nan;
  };

union ieee754_double
  {
    double d;

    /* This is the IEEE 754 double-precision format.  */
    struct
      {

    unsigned long int mantissa1:32;
    unsigned long int mantissa0:20;
    unsigned long int exponent:11;
    unsigned long int negative:1;
//# endif
//#endif              /* Little endian.  */
      } ieee;

    /* This format makes it easier to see if a NaN is a signalling NaN.  */
    struct
      {

    /* Together these comprise the mantissa.  */
    unsigned long int mantissa1:32;
    unsigned long int mantissa0:19;
    unsigned long int quiet_nan:1;
    unsigned long int exponent:11;
    unsigned long int negative:1;

      } ieee_nan;
  };
#endif

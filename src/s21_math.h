// Copyright 2021 rmarchel, jjalikak, rbabara//

#ifndef __S_21_MATH_H__
#define __S_21_MATH_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef EPS
#define EPS 1e-10
#endif
#ifndef SMALL_EPS
#define SMALL_EPS 1e-15
#endif
#ifdef NAN
#undef NAN
#endif
#ifndef NAN
#define NAN s21_nan()
#endif
#ifndef INFINITY
#define INFINITY s21_inf()
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623158e308
#endif
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

#if defined _MSC_VER
typedef unsigned __int64 uint64;
#else
typedef uint64_t uint64;
#endif

typedef struct deconstructed_double {
  int sign;
  int32_t exp;
  uint64_t mantiss;
} de_double;

typedef union {
  uint64 u;
  double f;
} ieee754;

int s21_isinf(double x);
int s21_isnan(double x);
double s21_nan(void);
double s21_inf(void);

int s21_abs(int x);
long double s21_fabs(double x);
long double s21_sqrt(double x);
long double s21_norm_sqrt(double x);

long double s21_sin(double x);
long double s21_cos(double x);
long double s21_tan(double x);
long double s21_asin(double x);
long double s21_acos(double x);
long double s21_atan(double x);

long double s21_log(double x);
long double normal_log(double num);
long double s21_pow(double base, double exp);

de_double disassemble_double(double num);
double normalize_double(double num, int* exp);
// double assemble_double(de_double ded);
double shift(double num, int shift);

long double s21_ceil(double x);
long double s21_exp(double x);
long double s21_floor(double x);
long double s21_fmod(double x, double y);

#endif
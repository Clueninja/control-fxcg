// Copyright 2021 rmarchel, jjalikak, rbabara//
// Copyright 2023 clueninja//

#ifndef __S_21_MATH_H__
#define __S_21_MATH_H__

#include "../../../include/stdint.h"
#include "../../../include/stdlib.h"

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
#define NAN (0.0/0.0)
#endif
#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif
#ifndef NEG_INFINITY
#define NEG_INFINITY (-1.0/0.0)
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
#ifndef M_LOG10
#define M_LOG10 2.3025850929940456840179914546844
#endif
#ifndef M_E
#define M_E 2.718281828
#endif

#define abs(x) ((x)<0? -(x): (x))
#define sqr(x) ((x)*(x))

#if defined _MSC_VER
typedef unsigned __int64 uint64;
#else
typedef uint64_t uint64;
#endif


int s21_isinf(double x);
int s21_isnan(double x);

double s21_sqrt(double x);
long double s21_bin_sqrt(double x);

long double s21_sin(double x);
long double s21_cos(double x);
long double s21_tan(double x);
long double s21_asin(double x);
long double s21_acos(double x);
long double s21_atan(double x);
// TODO: s21_atan2(y,x)

long double s21_log(double x);
long double s21_pow(double base, double exp);

long double s21_ceil(double x);
long double s21_exp(double x);
long double s21_floor(double x);
long double s21_fmod(double x, double y);

double s21_exp10(double x);
double s21_log10(double x);

#endif
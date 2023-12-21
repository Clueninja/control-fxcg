// Copyright 2021 rmarchel, jjalikak, rbabara//
// Copyright 2023 clueninja//

#include "s21_math.h"

int s21_isinf(double x) {
  return x == INFINITY;
}

int s21_isnan(double x) {
  return x == NAN;
}

long double s21_norm_sqrt(double x) {
  double S = x, a = 1, b = x;
  long double result_value = 0.0;
  while (abs(a - b) > EPS) {
    a = (a + b) / 2;
    b = S / a;
  }
  result_value = (a + b) / 2;
  return result_value;
}

long double s21_bin_sqrt(double x) {
  double a = 1, b = (x - 1) / 2;
  while (abs(a * a - x) > SMALL_EPS) {
    if (a * a < x) {
      a += b;
    } else {
      a -= b;
    }
    b /= 2;
  }
  return a;
}


double s21_sqrt(double S){
  if (S<=0.) return 0.;

  double x_0 = S/2.0, e = S, eps = 1e-6;
  while (abs(e) > eps){
    x_0 = (S/x_0 + x_0)/2.0;
    e = S - sqr(x_0);
  }
  return x_0;
}

double s21_log10(double num){
  return s21_log(num)/M_LOG10;
}
double s21_exp10(double exp){
  exp *= M_LOG10;
  return s21_exp(exp);
}

long double s21_log(double num) {
  if (num<0)
    return 0;
  long double x = (num - 1.) / (num + 1.), ai = 2. * x, sum = ai, d = x * x, eps = 1e-6;
  for (int i = 3; abs((double)ai) > eps; i += 2) {
    ai *= d * (i - 2.) / (long double)i;
    sum += ai;
  }
  return sum;
}

long double s21_exp(double x) {
  long double result = 1, temporary = 1, count = 1, abs_x = abs(x), eps = 1e-6;
  
  while (abs(result) > eps) {
    result *= abs_x / count;
    count += 1;
    temporary += result;
    if (temporary > DBL_MAX) {
      temporary = INFINITY;
      break;
    }
  }
  temporary = (x<0.) ? (temporary > DBL_MAX ? 0. : 1. / temporary) : temporary;
  return temporary = ((temporary > DBL_MAX) ? INFINITY : temporary);
}

long double s21_pow(double base, double ex) {
  long double result = 1.;
  if (s21_isnan(base) || s21_isnan(ex)) {
    result = NAN;
  } else if (ex == NEG_INFINITY) {
    result = 0.;
  } else {
    if ((double)(int64_t)ex == ex) {
      // use faster algorithm if ex is an integer
      double x = 1.;
      if (ex > 0) {
        for (int i = 0; i < ex; ++i) x *= base;
      } else if (ex < 0.) {
        for (int i = 0; i > ex; --i) x *= base;
        x = 1. / x;
      } else {
        x = 1.;
      }
      result = x;
    } else {
      result = s21_exp((double)(s21_log(base) * ex));
    }
  }
  return result;
}

long double s21_ceil(double x) {
  long double result = NAN;
  if (!s21_isnan(x)) {
    if (s21_isinf(x)) {
      result = x;
    } else {
      if (x < 0. || (int64_t)x == x) x -= 1.;
      result = (double)(int64_t)(x + 1.);
    }
  }
  return result;
}

long double s21_floor(double x) {
  long double result = NAN;
  if (!s21_isnan(x)) {
    if (s21_isinf(x)) {
      result = x;
    } else {
      long double result_buffer = (long double)(double)(int64_t)(x);
      if ((x < 0.) && (result_buffer != x)) x -= 1.;
      result = (long double)(double)(int64_t)(x);
    }
  }
  return result;
}

long double s21_fmod(double x, double y) {
  long double result;
  if (s21_isinf(y)) {
    result = x;
  } else if (s21_isnan(x) || s21_isnan(y) || s21_isinf(x) || y == 0) {
    result = NAN;
  } else {
    int n = (int)(x / y);
    result = (long double)x - (long double)(y * n);
  }
  return result;
}

long double s21_sin(double x) {
  long double eps = 1e-4;
  x = (double)s21_fmod(x, 2.0 * M_PI);
  double an = x, sum = an;
  for (double i = 3.; abs(an) > eps && i < 50.; i += 2.) {
    an *= -1. * x * x / (i - 1.) / i;
    sum += an;
  }
  return sum;
}

long double s21_cos(double x) {
  long double eps = 1e-4;
  x = (double)s21_fmod(x, 2.0 * M_PI);
  double an = 1., sum = an;
  for (double i = 2.; abs(an) > eps && i < 50.; i += 2.) {
    an *= -1. * x * x / (i - 1.) / i;
    sum += an;
  }
  return sum;
}
long double s21_tan(double x) { return s21_sin(x) / s21_cos(x); }


long double s21_asin(double x) {
  long double result = 0.0, eps = 1e-6;
  if ((x > 1) || (x < -1)) {
    result = 0. / 0.;
  } else if (x == 1 || x == -1) {
    result = M_PI_2;
    if (x < 0) result *= -1.;
  } else {
    long double temporary_result = 0.1;
    while (abs((double)temporary_result) > eps) {
      temporary_result =
          (x - s21_sin((double)result)) / s21_cos((double)result);
      result += temporary_result;
    }
  }
  return result;
}

long double s21_acos(double x) {
  long double result = 1.0, eps = 1e-3;
  if ((x > 1) || (x < -1)) {
    result = 0. / 0.;
  } else if (x == 1 || x == -1) {
    result = 0;
    if (x < 0) result = M_PI;
  } else {
    long double temporary_result = 0.1;
    int count = 0;
    while (abs((double)temporary_result) > eps && count < 20) {
      temporary_result = (s21_cos((double)result) - x) / s21_sin((double)result);
      result += temporary_result;
      count ++;
    }
  }
  return result;
}

long double s21_atan(double x) {
  long double result = 0.0;
  if (s21_isnan(x))
    result = 0. / 0.;
  else if (x == 0.) 
    result = 0.0;
  else if(x>100.0)
    result =  M_PI_2;
  else if(x<-100.0)
    result = -M_PI_2;
  else if (x > 0.0) 
    result = s21_acos(1. / s21_sqrt(1. + sqr(x)) );
  else 
    result = - s21_acos(1. / s21_sqrt(1. + sqr(x)) );
  
  return result;
}
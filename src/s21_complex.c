#include "s21_complex.h"
#include "s21_math.h"

complex_t cmult(complex_t l, complex_t r){
    complex_t result;
    result.r = l.r * r.r - l.i * r.i;
    result.i = l.r * r.i + l.r * r.i;
    return result;
}

complex_t cdiv(complex_t l, complex_t r){
    complex_t result;
    result.r = (l.r * r.r + l.i*r.i)/(sqr(r.r) + sqr(r.i));
    result.i = (l.i*r.r - r.i * l.r)/(sqr(r.r) + sqr(r.i));
    return result;
}

complex_t fromd(double r, double i){
    complex_t result =  {.r = r, .i = i};
    return result;
}

complex_t cadd(complex_t l, complex_t r){
    complex_t result;
    result.r = l.r + r.r;
    result.i = l.i + r.i;
    return result;
}

complex_t csub(complex_t l, complex_t r){
    complex_t result;
    result.r = l.r- r.r;
    result.i = l.i - r.i;
    return result;
}

complex_t csqr(complex_t x){
    complex_t result;
    result.r = sqr(x.r) - sqr(x.i);
    result.i = 2.0 * x.i * x.r;
    return result;
}

double cmag(complex_t x){
    return s21_sqrt(sqr(x.r) + sqr(x.i));
}
double cphase(complex_t x){
    double a = s21_atan(abs(x.i)/abs(x.r));
    // TODO: Fix quadrants
    if (x.r>=0){
        if (x.i>=0)
            return a;
        else
            return -a;
    }
    else{
        if (x.i>=0)
            return M_PI - a;
        else
            return -M_PI + a;
    }
}

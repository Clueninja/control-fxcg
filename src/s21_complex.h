#ifndef S21_COMPLEX
#define S21_COMPLEX

typedef struct s_complex_t{
    double r;
    double i;
} complex_t;

complex_t cmult(complex_t l, complex_t r);

complex_t cdiv(complex_t l, complex_t r);

complex_t fromd(double r, double i);

complex_t cadd(complex_t l, complex_t r);

complex_t csub(complex_t l, complex_t r);

complex_t csqr(complex_t x);

double cmag(complex_t x);

double cphase(complex_t x);


#endif
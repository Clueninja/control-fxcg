#include "utils.h"
#include <string.h>

void fillArea(unsigned x,unsigned y,unsigned w,unsigned h,unsigned short col){
    unsigned short*s=(unsigned short*)GetVRAMAddress();
    s+=(y*384)+x;
    while(h--){
        unsigned w2=w;
        while(w2--)
            *s++=col;
        s+=384-w;
    }
}
void plot(unsigned x,unsigned y,unsigned short color){
    unsigned short*s=(unsigned short*) GetVRAMAddress();
    s+=(y*384)+x;
    *s=color;
}

//Function was originally written by Christopher “Kerm Martian” Mitchell.
void drawLine(int x1, int y1, int x2, int y2, unsigned short color) {
    signed char ix;
    signed char iy;
 
    // if x1 == x2 or y1 == y2, then it does not matter what we set here
    int delta_x = (x2 > x1?(ix = 1, x2 - x1):(ix = -1, x1 - x2)) << 1;
    int delta_y = (y2 > y1?(iy = 1, y2 - y1):(iy = -1, y1 - y2)) << 1;
 
   plot(x1, y1, color);  
    if (delta_x >= delta_y) {
        int error = delta_y - (delta_x >> 1);        // error may go below zero
        while (x1 != x2) {
            if (error >= 0) {
                if (error || (ix > 0)) {
                    y1 += iy;
                    error -= delta_x;
                }                           // else do nothing
         }                              // else do nothing
            x1 += ix;
            error += delta_y;
            plot(x1, y1, color);
        }
    } else {
        int error = delta_x - (delta_y >> 1);      // error may go below zero
        while (y1 != y2) {
            if (error >= 0) {
                if (error || (iy > 0)) {
                    x1 += ix;
                    error -= delta_y;
                }                           // else do nothing
            }                              // else do nothing
            y1 += iy;
            error += delta_x;  
            plot(x1, y1, color);
        }
    }
}

// TODO: adapt precision flexibily
char * _float_to_char(float x, char *p, int str_len) {
    memset(p, ' ', str_len);
    char *s = p + str_len; // go to end of buffer
    *s = '\0';

    int decimals;  // variable to store the decimals
    int units;  // variable to store the units (part to left of decimal place)
    if (x < 0) { // take care of negative numbers
        decimals = (int)(x * -10) % 10; // make 1000 for 3 decimals etc.
        units = (int)(-1 * x);
    } else { // positive numbers
        decimals = (int)(x * 10) % 10;
        units = (int)x;
    }

    //*--s = (decimals % 10) + '0';
    //decimals /= 10; // repeat for as many decimal places as you need
    *--s = (decimals % 10) + '0';
    *--s = '.';

    do {
        *--s = (units % 10) + '0';
        units /= 10;
    }while (units > 0);

    if (x < 0) *--s = '-'; // unary minus sign for negative numbers
    return s;
}

//Function was originally written by Christopher “Kerm Martian” Mitchell.
short unsigned int heightcolor(float z, float z_min, float z_max) {
         float frac = ((z-z_min)/(z_max-z_min));
         
         //color!
         float r = (0.25f)-frac;
         float g = (0.5f)-frac;
         float b = (0.75f)-frac;

         //calculate the R/G/B values
         r = (r>0.f)?r:-r; g = (g>0.f)?g:-g; b = (b>0.f)?b:-b;   //absolute value
         r = (0.25f)-r; g = (1.f/3.f)-g; b = (0.25f)-b;   //invert
         r = (r>0.f)?(6.f*r):0.f; g = (g>0.f)?(6.f*g):0.f; b = (b>0.f)?(6.f*b):0.f;   //scale the chromatic triangles
         r = (r>1.f)?1.f:r; g = (g>1.f)?1.f:g; b = (b>1.f)?1.f:b;   //clip the top of the chromatic triangles
         if (frac < 0.25f) r = (r+1.f)/2.f;   //adjust the bottom end of the scale so that z_min is red, not black
         if (frac > 0.75f) b = (b+1.f)/2.f;   //adjust the top end of the scale so that z_max is blue, not black
         return (short unsigned int)(0x0000ffff & (((int)(31.f*r) << 11) | ((int)(63.f*g) << 5) | ((int)(31.f*b))));   //put the bits together
}
int map(int x, int a, int b, int min, int max){
    return min + (x-a)/(b-a) * (max-min);
}

double mapd(double x, double a, double b, double min, double max){
    x = max(min(x,b), a);
    return min + (x-a)/(b-a) * (max-min);
}
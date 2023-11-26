#pragma once

#include "../../../include/fxcg/display.h"
#include "../../../include/math.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void fillArea(unsigned x,unsigned y,unsigned w,unsigned h,unsigned short col);
void plot(unsigned x,unsigned y,unsigned short color);

void drawLine(int x1, int y1, int x2, int y2, unsigned short color);
char * _float_to_char(float x, char *p, int str_len);

int map(int x, int a, int b, int min, int max);
double mapd(double x, double a, double b, double min, double max);

#include "../../../include/fxcg/display.h"
#include "../../../include/fxcg/keyboard.h"
#include "../../../include/stdio.h"
#include "../../../include/stdlib.h"
#include "../../../include/string.h"

#include "menu.h"
#include "utils.h"
#include "s21_math.h"
#include "s21_complex.h"


#define MINI_WIDTH 7
#define MINI_HEIGHT 12

#define STEP_AXIS_GAP_X (MINI_WIDTH * 4)
#define STEP_AXIS_GAP_Y 25
#define STEP_AXIS_WIDTH (LCD_WIDTH_PX - STEP_AXIS_GAP_X)
#define STEP_AXIS_HEIGHT (LCD_HEIGHT_PX - STEP_AXIS_GAP_Y)

#define BODE_AXIS_GAP_X (MINI_WIDTH * 4)
#define BODE_AXIS_GAP_Y 25
#define BODE_AXIS_WIDTH (LCD_WIDTH_PX - BODE_AXIS_GAP_X-1)
#define BODE_AXIS_HEIGHT ((LCD_HEIGHT_PX - BODE_AXIS_GAP_Y)/2)
#define BODE_AXIS_END (LCD_HEIGHT_PX - BODE_AXIS_GAP_Y)


// might split bode into gain and phase
static struct menu_tab Tab0 = {"First", 4, {{1, "Edit"}, {2, "Step Response"}, {3, "Bode Plot"},{4, "Characteristics"}}};
static struct menu_tab Tab1 = {"Second", 4, {{11, "Edit"}, {12, "Step Response"}, {13, "Bode Plot"}, {14, "Characteristics"}}};
static struct menu_tab Tab2 = {"Bode", 3, {{21, "Edit"}, {22, "Bode Plot"}, {23, "Characteristics"}}};

static struct menu_page Page0 = {"OPT", KEY_CTRL_OPTN, 3, {&Tab0, &Tab1, &Tab2}};

static struct Tmenu geometry = {1, {&Page0}};



// could generalise here maybe useful for a library??

// first order  1/(as + b)
static struct {unsigned char edit[256]; double a,b;} first;

// second order 1/(a s^2 + b s + c)
static struct {unsigned char edit[256]; double a,b,c;} second;

// bode plot (a0 )
static struct {unsigned char edit_poles[256]; double poles[256];} bode;

enum plot_type{
    FIRST, SECOND, BODE
};

enum plot_mode{
    DOTTED, LINE
};



//static char empty_buffer[256];

//static char * empty_text = "--";

const char * text_edit = "--Denominator:";



void set_default_values(){
    first.a = 1.;
    first.b = 1.;

    second.a = 1.;
    second.b = 1.;
    second.c = 1.;

    bode.poles[0] = -1.;
    bode.poles[1] = -2.;
}


int scaled_value(int start, int length, double min_x, double max_x, double x){
    // map x and y between min and max
    x = min(max(x, min_x), max_x);

    x = (x - min_x)/(max_x-min_x) * length + start;
    return (int)x;
}

void draw_step_axis(double start_t, double end_t, double step_t, double min_e, double max_e, double step_e){
    Bdisp_AllClr_VRAM();

    // Draw grid lines
    for (double y = 0.; y<max_e; y += step_e){
        int scaled_y = STEP_AXIS_HEIGHT - scaled_value(0, STEP_AXIS_HEIGHT, 0 , max_e, y);
        for (int x = 0; x<STEP_AXIS_WIDTH; x++){
            plot(x+STEP_AXIS_GAP_X, scaled_y, COLOR_BLUE);
        }
    }
    

    for (double x = start_t ; x<end_t; x += step_t){
        int scaled_x = scaled_value(STEP_AXIS_GAP_X, STEP_AXIS_WIDTH, start_t , end_t, x);
        for (int y = 0; y<STEP_AXIS_HEIGHT; y++){
            plot(scaled_x, y, COLOR_BLUE);
        }
    }

    for (int y=0; y<STEP_AXIS_HEIGHT; y++){
        plot(STEP_AXIS_GAP_X, y, COLOR_BLACK);
    }
    for (int x = 0; x<STEP_AXIS_WIDTH; x++) {
        plot(x+STEP_AXIS_GAP_X, STEP_AXIS_HEIGHT, COLOR_BLACK);
    }
    

    char buffer[6] = {};
    // display y axis
    _float_to_char(min_e, buffer, 4);
    int char_x=0, char_y=STEP_AXIS_HEIGHT - MINI_HEIGHT - 24;
    PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

    memset(buffer, 0, 6);
    _float_to_char(max_e, buffer, 4);
    char_x=0, char_y=0;
    PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

    memset(buffer, 0, 6);
    _float_to_char(start_t, buffer, 5);
    char_x=STEP_AXIS_GAP_X - MINI_WIDTH*2, char_y=STEP_AXIS_HEIGHT - 22;
    PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

    memset(buffer, 0, 6);
    _float_to_char(end_t, buffer, 5);
    char_x=STEP_AXIS_WIDTH - STEP_AXIS_GAP_X + MINI_WIDTH * 2, char_y=STEP_AXIS_HEIGHT -22 ;
    PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

}

// Fix gain and phase for BODE Plot type
double calculate_bode_gain(enum plot_type type, double w){
    switch (type) {
    case FIRST:
    {
        double a = first.a, b = first.b;
        return 20. *s21_log10(s21_sqrt(sqr(b) + sqr(a*w))/(sqr(b) + sqr(a*w)));
    }
    // TODO: Fix bug with underdamped systems
    case SECOND:
    {
        double a=second.a, b=second.b, c=second.c;
        double w0 = 1.0/ s21_sqrt(a/c);
        double zeta = b/c/2.0 * w0;
        complex_t den;
        den.r = (1.0 - sqr(w/w0));
        den.i = (2.0 * zeta *w/w0);
        return 20. * s21_log10(1.0/c/cmag(den));
    }
    case BODE:
    {
        complex_t poles[10] = {};
        int n;
        for (n =0; bode.poles[n]!=0 && n<10; n++){
            poles[n].r = -bode.poles[n];
            poles[n].i = w;
        }
        double product = 1.;
        for (int i = 0; i<n; i++){
            product *= cmag(poles[i]);
        }
        return 20. * s21_log10(1./product);
    }
    }
    return 0.;
}

double calculate_bode_phase(enum plot_type type, double w){
    switch (type) {
    case FIRST:
    {
        double a = first.a, b = first.b;
        return -s21_atan((a*w)/(b));
    }
    // check quadrants :) 
    case SECOND:
    {
        double a=second.a, b=second.b, c=second.c;
        double w0 = 1.0/ s21_sqrt(a/c);
        double zeta = b/c/2.0 * w0;
        complex_t den;
        den.r = (1.0 - sqr(w/w0));
        den.i = (2.0 * zeta *w/w0);
        return -cphase(den);
    }
    case BODE:
    {
        complex_t poles[10] = {};
        int n;
        for (int n =0; bode.poles[n]!=0 && n<10; n++){
            // 1/((s+1) * (s+2) * ...)
            poles[n].r = -bode.poles[n];
            poles[n].i = w;
        }
        double sum = 0.;
        for (int i = 0; i<=n; i++){
            sum += cphase(poles[i]);
        }
        return -sum;
    }
    }
    return 0.;
}

double calculate_step_gain(enum plot_type type, double t){
    switch(type)
    {
        case FIRST:
        {
            double a = first.a, b = first.b;
            double g = 1./b; double tau = a/b;
            return  g - g * s21_exp(-t/tau);
        }

        case SECOND:
        {
            double a=second.a, b=second.b, c=second.c;
            double w0 = 1.0/ s21_sqrt(a/c);
            double zeta = b/c/2.0 * w0;
            // zeta = 0
            if (zeta == 0.0)
                return 1.0/c * (1.0 - s21_cos(w0 * t));
            
            //small fix for zeta < 1
            if (zeta < 0.95){
                double temp = s21_sqrt(1.0 - sqr(zeta));
                return 1.0/c * (1.0 - s21_exp(- zeta * w0 *t)/temp * s21_sin(temp * w0 * t + s21_atan(temp / zeta)));
            }
            // zeta ~= 1
            if (zeta <= 1.)
                return 1.0/c * (1.0 - (1 + w0*t) * s21_exp(-w0 * t));
            // zeta > 1
            double temp = s21_sqrt(sqr(zeta)- 1.0);
            double atanh_temp = 1.0/2.0 * s21_log((1.0 + temp/zeta)/(1.0 - temp/zeta));
            return 1.0/c * (1.0 - s21_exp(- zeta * w0 *t)/ temp * (s21_exp(temp*w0*t + atanh_temp) - s21_exp(-temp*w0*t - atanh_temp))/2.0 );
        }
        case BODE:
        {
            return 0.;
        }
    }
    return 0.;
}

// TODO: Validate graph is perfect
void plot_step(enum plot_type graph_plot, enum plot_mode graph_mode){
    double start_t = 0.;
    double end_t = 10.;

    double e [STEP_AXIS_WIDTH];
    double t [STEP_AXIS_WIDTH];
    

    for(;;){
        // fill array with graph data
        for (int x = 0; x<STEP_AXIS_WIDTH; x++){
            t[x] = (end_t - start_t) * ((double)x) / ((double)STEP_AXIS_WIDTH) + start_t;
            e[x] = calculate_step_gain(graph_plot, t[x]);
        }

        // calculate max_e
        double max_e = 0;
        for (int i = 0; i<STEP_AXIS_WIDTH; i++){
            max_e = (e[i]>max_e)? e[i]: max_e;
        }
        max_e *=1.2;


        // calculate step_e for grid lines

        draw_step_axis(start_t, end_t, 1., 0, max_e, 0.2);

        
        int old_y=0;
        for(int x = 0; x < STEP_AXIS_WIDTH; x++){
            int y = STEP_AXIS_HEIGHT - scaled_value(0, STEP_AXIS_HEIGHT, 0, max_e, e[x]);
            switch(graph_mode){
            case DOTTED:
                plot(x + STEP_AXIS_GAP_X
            ,y,COLOR_RED);
                break;
            case LINE:
                drawLine((x-1), old_y, x, y, COLOR_RED);
                break;
            }
            old_y = y;
        }

        int key;
        GetKey(&key);
        if (key == KEY_CTRL_EXE || key == KEY_CTRL_EXIT)
            break;
        if (key == KEY_CTRL_RIGHT){
            start_t += 10.;
            end_t += 10.;
        }
        if (key == KEY_CTRL_LEFT){
            start_t -= 10.;
            end_t -= 10.;
        }

    }

}

// TODO: Fix Bode plot
void plot_bode(enum plot_type graph_plot, enum plot_mode graph_mode){
    // when the scale = 0 and pixel fraction = 0.5, w = 3ish
    // when the scale = -1 and pixel fraction = 0.5 w = 0.3...
    double exp = -1.0; // 0.1 ... 1

    double w;
    double g[BODE_AXIS_WIDTH];
    double p[BODE_AXIS_WIDTH];

    // perform analysis
    double max_g = 0., min_g = -20.;

    double max_p = 0, min_p = -M_PI;

    int y, old_y, old_y2;
    old_y = 0;
    old_y2 = BODE_AXIS_END;
    for(;;){
        for (int x = 0; x< BODE_AXIS_WIDTH; x++){
            
            w = s21_exp10(((double) x)/((double) BODE_AXIS_WIDTH) + exp);

            g[x] = calculate_bode_gain(graph_plot, w);
            p[x] = calculate_bode_phase(graph_plot, w);
        }
        

        /*
        for (int i = 0; i<BODE_AXIS_WIDTH; i++){
            max_g = g[i] > max_g ? g[i] : max_g;
            min_g = g[i] < min_g ? g[i] : min_g;

            max_p = p[i] > max_p ? p[i] : max_p;
            min_p = p[i] < min_p ? p[i] : min_p;
        }
        */

       Bdisp_AllClr_VRAM();
        // draw vertical grid lines
        for (double x = 1.0 ; x<=10.0; x +=1.0){
            int scaled_x = (int)((double) BODE_AXIS_WIDTH * s21_log10(x) )+ BODE_AXIS_GAP_X;
            for (int y = 0; y<BODE_AXIS_END; y++){
                plot(scaled_x, y, COLOR_LIGHTSLATEGRAY);
            }
        }

        // draw gain grid lines at 0dB, -3dB, -10dB and -20dB
        double hor_lines [] = {0., -3., -10., -20.};
        for (int i = 0; i<=3; i++){
            int pixel_y = (int) mapd(hor_lines[i], min_g, max_g, (double) BODE_AXIS_HEIGHT, 0);
            for (int x = 0; x<BODE_AXIS_WIDTH; x++){
                plot(x+BODE_AXIS_GAP_X, pixel_y, COLOR_LIGHTSLATEGRAY);
            }
        }


        // draw phase grid lines
        double step_p = M_PI_4/2;
        for (double grid_p = min_p ; grid_p<max_p; grid_p += step_p){
            int scaled_y = mapd(grid_p, min_p, max_p, (double) BODE_AXIS_HEIGHT, (double) BODE_AXIS_END);
            for (int x = 0; x<BODE_AXIS_WIDTH; x++){
                plot(x+BODE_AXIS_GAP_X, scaled_y, COLOR_LIGHTSLATEGRAY);
            }
        }

        // draw axis lines
        for (int y=0; y<BODE_AXIS_END; y++){
            plot(BODE_AXIS_GAP_X, y, COLOR_BLUE);
        }

        for (int x = 0; x<BODE_AXIS_WIDTH; x++) {
            plot(x+BODE_AXIS_GAP_X, 0, COLOR_BLUE);
            plot(x+BODE_AXIS_GAP_X, BODE_AXIS_HEIGHT, COLOR_BLUE);
            //plot(x, 3*LCD_HEIGHT_PX/4, COLOR_LIGHTSLATEGRAY);
            plot(x+BODE_AXIS_GAP_X, BODE_AXIS_END, COLOR_BLUE);
        }

        int char_x,char_y;
        char buffer[7] = {};

        // display gain values
        memset(buffer, 0, 6);
        _float_to_char(-3.0, buffer, 6);
        char_x=0, char_y=0 ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        memset(buffer, 0, 6);
        _float_to_char(min_g, buffer, 6);
        char_x=0, char_y= BODE_AXIS_HEIGHT-24-MINI_HEIGHT ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);
        
        // display phase values
        memset(buffer, 0, 6);
        _float_to_char(max_p, buffer, 6);
        char_x=0, char_y= BODE_AXIS_HEIGHT-24 ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        memset(buffer, 0, 6);
        _float_to_char(min_p, buffer, 6);
        char_x=0, char_y= BODE_AXIS_END - MINI_HEIGHT -24;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        // display omega
        memset(buffer, 0, 6);
        _float_to_char(s21_exp10(exp), buffer, 6);
        char_x=MINI_WIDTH*2, char_y= BODE_AXIS_END-22 ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        memset(buffer, 0, 6);
        _float_to_char(s21_exp10(exp+1.0), buffer, 6);
        char_x=BODE_AXIS_WIDTH - 2*MINI_WIDTH, char_y= BODE_AXIS_END-22;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);


        // draw gain
        // TODO: display in decibels
        for(int x = 0; x< BODE_AXIS_WIDTH; x++){
            y = (int) mapd(g[x], min_g, max_g,(double) BODE_AXIS_HEIGHT, 0);
            switch (graph_mode) {
            case (DOTTED):
                plot(x+BODE_AXIS_GAP_X,y,COLOR_RED);
                break;
            case LINE:
                drawLine((x-1), old_y, x, y, COLOR_RED);
                break;
            }
            old_y = y;
        }
        //draw phase
        for (int x = 0; x< BODE_AXIS_WIDTH; x++){
            y = (int) mapd(p[x], min_p, max_p, (double) BODE_AXIS_END, (double) BODE_AXIS_HEIGHT);
            switch (graph_mode) {
            case DOTTED:
                plot(x + BODE_AXIS_GAP_X,y,COLOR_BLACK);
                break;
            case LINE:
                drawLine((x-1), old_y2, x, y, COLOR_BLACK);
                break;
            }
            old_y2 = y;
        }

        int key;
        GetKey(&key);
        if (key == KEY_CTRL_EXE || key == KEY_CTRL_EXIT)
            break;
        if (key == KEY_CTRL_RIGHT){
            exp++;
        }
        if (key == KEY_CTRL_LEFT){
            exp--;
        }
        /*
        if (key == KEY_CTRL_UP){
            min_g += 20;
            max_g += 20;
        }
        if (key == KEY_CTRL_DOWN){
            min_g -= 20;
            max_g -=20;
        }
        */
    }
}

void text_write_buffer(char * buffer, const char * text){
    int key;
    int cursor=0, start = 0;
    // write text into numerator/ denominator
    DisplayMBString((unsigned char*)buffer, start, cursor, 1, 2);

    PrintXY(1,1,text,TEXT_MODE_NORMAL,TEXT_COLOR_BLACK);

    for(;;)
    {
        GetKey(&key); // Blocking is GOOD.  This gets standard keys processed and, possibly, powers down the CPU while waiting
        if(key == KEY_CTRL_EXE || key == KEY_CTRL_EXIT || key == KEY_CTRL_OPTN)
            break;

        if(key && key < 30000)
        {
            cursor = EditMBStringChar((unsigned char*)buffer, 256, cursor, key);
            DisplayMBString((unsigned char*)buffer, start, cursor, 1,2);
        }
        else
            EditMBStringCtrl((unsigned char*)buffer, 256, &start, &cursor, &key, 1, 2);
    }
    Bdisp_AllClr_VRAM();
    Cursor_SetFlashOff();
}

// maybe not the best solution but it works and seems safe...ish
void str_to_array(unsigned char * str, int array_len, double array[]){
    // Endianness??
    for(int n =0; str[n] != '\0'; n++){
        if (str[n] == KEY_CHAR_MINUS || str[n] == KEY_CHAR_PMINUS){
            str[n] = '-';
            //str[n+1] = '-';
        }
    }
    unsigned char * start = str;
    unsigned char * end = str;
    for (int i = 0; i<array_len && *start != '\0'; i++){
        array[i] = strtod((char *)start, (char **) &end);
        // Hacky solution to remove a single whitespace or comma thats seperating numbers
        while (*end == ',' || *end == ' ') end++;
        start = end;
    }
}

void warning_screen(void){
    Bdisp_AllClr_VRAM();
    PrintXY(2, 2, "--Warning:", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
    PrintXY(2, 3,"--The values you have", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
    PrintXY(2, 4,"--entered diverge.", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
    PrintXY(2, 5, "--Press 'AC' to", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
    PrintXY(2, 6, "--acknowledge", TEXT_MODE_NORMAL, TEXT_COLOR_RED);
    int key;
    do{
        GetKey(&key);
    }while(key != KEY_CTRL_AC);
}

int main(void){

    int choice;
    int tab = 0;
    set_default_values();
    char buffer[256] = {}; int n;

    // set cursor for writing text

    for(;;){
        // clear display
        Bdisp_AllClr_VRAM();

        // get option from user
        choice = menu(&geometry, 0, tab);

        switch(choice){
            case 1:
            {
                text_write_buffer((char * )first.edit, text_edit);
                str_to_array(first.edit, 2, &first.a);
                if (first.b/first.a < 0.){
                   warning_screen();
                }
                break;
            }
            case 2:
            {
                plot_step(FIRST, DOTTED);
                break;
            }   
            case 3:
            {
                plot_bode(FIRST, DOTTED);
                break;
            }
            case 4:
            {
                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Time Constant: ");
                _float_to_char(first.a/first.b, buffer + n-1, 5);
                PrintXY(1, 1, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Steady State: ");
                _float_to_char(1./first.b, buffer + n-1, 5);
                PrintXY(1, 3, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Rise Time: ");
                _float_to_char(2.2 * first.a/first.b, buffer + n-1, 5);
                PrintXY(1, 5, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Settling Time: ");
                _float_to_char(4 * first.a/first.b, buffer + n-1, 5);
                PrintXY(1, 7, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                GetKey(NULL);
                break;
            }

            case 11:
            {
                text_write_buffer((char *) second.edit, text_edit);
                str_to_array(second.edit, 3, &second.a);
                // check its not divergent
                if (second.c/ second.a <0.){
                    warning_screen();
                }
                break;
            }
            case 12:
            {
                plot_step(SECOND, DOTTED);
                break;
            }
            case 13:
            {
                plot_bode(SECOND, DOTTED);
                break;
            }
            case 14:
            {
                double wn = s21_sqrt(second.c/second.a);
                double zeta = second.b/(2.*s21_sqrt(second.a*second.c));
                double sigma = zeta * wn;
                double wd = wn * s21_sqrt(1.0-sqr(zeta));

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--N Frequency: ");
                _float_to_char(wn, buffer + n-1, 5);
                PrintXY(1, 1, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
                
                memset(buffer, 0, 256);
                n = sprintf(buffer, "--D Frequency: ");
                _float_to_char(wd, buffer + n-1, 5);
                PrintXY(1, 2, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Damping Ratio: ");
                _float_to_char(zeta, buffer + n-1, 5);
                PrintXY(1, 3, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Sigma: ");
                _float_to_char(sigma, buffer + n-1, 5);
                PrintXY(1, 4, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Rise Time: ");
                _float_to_char(1.8/wn, buffer + n-1, 5);
                PrintXY(1, 5, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Settling Time: ");
                _float_to_char(4.6/(zeta*wn), buffer + n-1, 5);
                PrintXY(1, 6, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Settling Value: ");
                _float_to_char(1/second.c, buffer + n-1, 5);
                PrintXY(1, 7, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                if (zeta>0 && zeta<1){
                    memset(buffer, 0, 256);
                    n = sprintf(buffer, "--Overshoot: ");
                    _float_to_char(s21_exp((-M_PI * zeta) / s21_sqrt(1-sqr(zeta))), buffer + n-1, 5);
                    PrintXY(1, 8, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
                }

                GetKey(NULL);
                break;
            }

            case 21:
            {
                memset(bode.poles, 0, 256 * sizeof(double));
                text_write_buffer((char *) bode.edit_poles, "--Poles");
                str_to_array(bode.edit_poles, 256, bode.poles);
                break;
            }
            case 22:
            {
                plot_bode(BODE, DOTTED);
                break;
            }
            case 23:
            {
                memset(buffer, 0, 256);
                n = sprintf(buffer, "--Poles:");
                PrintXY(5, 1, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

                int n_poles =0;
                for (n_poles = 0; n_poles < 7; n_poles++){
                     memset(buffer, 0, 256);
                     n = sprintf(buffer, "--");
                    _float_to_char(bode.poles[n_poles], buffer+n-1, 6);
                    PrintXY(1, 2+n_poles, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
                }
                GetKey(NULL);
                break;
            }
                
            default:
                break;
        }

        tab = choice / 10;
        
    }
    return 0; 
}

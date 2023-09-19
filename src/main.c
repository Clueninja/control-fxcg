#include "../../../include/fxcg/display.h"
#include "../../../include/fxcg/keyboard.h"
#include "../../../include/stdio.h"
#include "../../../include/stdlib.h"
#include "../../../include/string.h"

#include "menu.h"
#include "utils.h"
#include "s21_math.h"


#define MINI_WIDTH 7
#define MINI_HEIGHT 12

#define STEP_AXIS_GAP_X MINI_WIDTH * 4
#define STEP_AXIS_GAP_Y 25
#define STEP_AXIS_WIDTH LCD_WIDTH_PX - STEP_AXIS_GAP_X
#define STEP_AXIS_HEIGHT LCD_HEIGHT_PX - STEP_AXIS_GAP_Y

#define BODE_AXIS_GAP_X MINI_WIDTH * 4
#define BODE_AXIS_GAP_Y 25
#define BODE_AXIS_WIDTH LCD_WIDTH_PX - BODE_AXIS_GAP_X-1
#define BODE_AXIS_HEIGHT (LCD_HEIGHT_PX - BODE_AXIS_GAP_Y)/2
#define BODE_AXIS_END LCD_HEIGHT_PX - BODE_AXIS_GAP_Y





// might split bode into gain and phase
static struct menu_tab Tab0 = {"First", 4, {{1, "Edit"}, {2, "Step Response"}, {3, "Bode Plot"},{4, "Characteristics"}}};
static struct menu_tab Tab1 = {"Second", 4, {{11, "Edit"}, {12, "Step Response"}, {13, "Bode Plot"}, {14, "Characteristics"}}};
static struct menu_tab Tab2 = {"Bode", 2, {{21, "Edit"}, {22, "Bode Plot"}}};

static struct menu_page Page0 = {"OPT", KEY_CTRL_OPTN, 3, {&Tab0, &Tab1, &Tab2}};

static struct Tmenu geometry = {1, {&Page0}};



// could generalise here maybe useful for a library??

// first order  1/(as + b)
static struct {char edit[256]; double a,b;} first;

// second order 1/(a s^2 + b s + c)
static struct {char edit[256]; double a,b,c;} second;

// bode plot (a0 )
static struct {char edit[256]; double transform[256];} bode;

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

double second_bode_gain(double w){
    double a=second.a, b=second.b, c=second.c;
    double num = s21_sqrt(sqr(c - a* sqr(w)) + sqr(b*w));
    double den = sqr(c-a* sqr(w)) + b * sqr(w);
    return num/den;
}
double second_bode_phase(double w){
    double a=second.a, b=second.b, c=second.c;
    return s21_atan(- (b*w)/(c - a* sqr(w)) );
}
double first_bode_gain(double w){
    double a = first.a, b = first.b;
    return s21_sqrt(sqr(b) + sqr(a*w))/(sqr(b) + sqr(a*w));
}
double first_bode_phase(double w){
    double a = first.a, b = first.b;
    return s21_atan(-(a*w)/(b));
}

double first_step(double t){
    double a = first.a, b = first.b;
    double g = 1/b; double tau = a/b;
    return  g - g * s21_exp(-t/tau);
}

double second_step(double t){
    double a=second.a, b=second.b, c=second.c;
    double wn = s21_norm_sqrt(c/a);
    double zeta = b/(2.*s21_norm_sqrt(a*c));
    double sigma = zeta * wn;
    double wd = wn * s21_norm_sqrt(1-sqr(zeta));
    return 1./c * (1. - (wn/wd) * s21_exp(-sigma * t ) * (s21_cos(wd*t) + sigma/wd * s21_sin(wd*t)));
}
// impossible but its here
double bode_step(double t){
    return 0.;
}

double step_function(double t){
    return (t<0)? 0: 1;
}


void plot_step(enum plot_type graph_plot, enum plot_mode graph_mode){
    double start_t = 0.;
    double end_t = 10.;

    double e [STEP_AXIS_WIDTH];
    double t [STEP_AXIS_WIDTH];
    

    for(;;){
        // fill array with graph data
        for (int x = 0; x<STEP_AXIS_WIDTH; x++){
            t[x] = (end_t - start_t) * ((double)x) / ((double)STEP_AXIS_WIDTH) + start_t;
            switch (graph_plot) {
            case FIRST:
                e[x] = first_step(t[x]);
                break;
            case SECOND:
                e[x] = second_step(t[x]);
                break;
            case BODE:
                e[x] = bode_step(t[x]);
                break;
            }
            
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

double round_to_closest_step(double value, double step){
    value = value + step/2.;
    return step * (int)(value / step);
}
void plot_bode(enum plot_type graph_plot, enum plot_mode graph_mode){
    double lower_w = 0.1, upper_w = 1;

    double w[BODE_AXIS_WIDTH];
    double g[BODE_AXIS_WIDTH];
    double p[BODE_AXIS_WIDTH];

    double step_w = 0.2;

    int y, old_y, old_y2;
    old_y = 0;
    old_y2 = BODE_AXIS_END;
    for(;;){

        for (int x = 0; x< BODE_AXIS_WIDTH; x++){
            // incorrect but simple
            w[x] = (upper_w-lower_w) * ((double)x) / ((double)BODE_AXIS_WIDTH) + lower_w;
            switch(graph_plot){
            case FIRST:
                g[x] = first_bode_gain(w[x]);
                p[x] = first_bode_phase(w[x]);
                break;
            case SECOND:
                g[x] = second_bode_gain(w[x]);
                p[x] = second_bode_phase(w[x]);
                break;
            case BODE:
                break;
            }
            
        }

        // perform analysis
        double max_g = 1., min_g = 0.;

        double max_p = M_PI_2, min_p = -M_PI_2;
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
        for (double x = upper_w ; x>=lower_w; x -= step_w){
            int scaled_x = scaled_value(BODE_AXIS_GAP_X, BODE_AXIS_WIDTH, lower_w , upper_w, x);
            for (int y = 0; y<BODE_AXIS_END; y++){
                plot(scaled_x, y, COLOR_LIGHTSLATEGRAY);
            }
        }

        // draw gain grid lines
        double step_g = 0.2;
        for (double grid_g = min_g ; grid_g<max_g; grid_g += step_g){
            int scaled_y = BODE_AXIS_HEIGHT - scaled_value(0, BODE_AXIS_HEIGHT, min_g , max_g, grid_g);
            for (int x = 0; x<BODE_AXIS_WIDTH; x++){
                plot(x+BODE_AXIS_GAP_X, scaled_y, COLOR_LIGHTSLATEGRAY);
            }
        }

        // draw phase grid lines
        double step_p = M_PI_4/2;
        for (double grid_p = min_p ; grid_p<max_p; grid_p += step_p){
            int scaled_y = BODE_AXIS_END - scaled_value(0, BODE_AXIS_HEIGHT, min_p , max_p, grid_p);
            for (int x = 0; x<BODE_AXIS_WIDTH; x++){
                plot(x+BODE_AXIS_GAP_X, scaled_y, COLOR_LIGHTSLATEGRAY);
            }
        }

        // draw axis lines
        for (int y=0; y<BODE_AXIS_END; y++){
            plot(BODE_AXIS_GAP_X, y, COLOR_GREEN);
        }

        for (int x = 0; x<BODE_AXIS_WIDTH; x++) {
            plot(x+BODE_AXIS_GAP_X, 0, COLOR_GREEN);
            plot(x+BODE_AXIS_GAP_X, BODE_AXIS_HEIGHT, COLOR_GREEN);
            //plot(x, 3*LCD_HEIGHT_PX/4, COLOR_LIGHTSLATEGRAY);
            plot(x+BODE_AXIS_GAP_X, BODE_AXIS_END, COLOR_GREEN);
        }

        int char_x,char_y;
        char buffer[6] = {};

        // display gain values
        memset(buffer, 0, 6);
        _float_to_char(max_g, buffer, 5);
        char_x=0, char_y=0 ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        memset(buffer, 0, 6);
        _float_to_char(min_g, buffer, 5);
        char_x=0, char_y= BODE_AXIS_HEIGHT-24-MINI_HEIGHT ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);
        
        // display phase values
        memset(buffer, 0, 6);
        _float_to_char(max_p, buffer, 5);
        char_x=0, char_y= BODE_AXIS_HEIGHT-24 ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        memset(buffer, 0, 6);
        _float_to_char(min_p, buffer, 5);
        char_x=0, char_y= BODE_AXIS_END - MINI_HEIGHT -24;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        // display omega
        memset(buffer, 0, 6);
        _float_to_char(lower_w, buffer, 6);
        char_x=MINI_WIDTH*2, char_y= BODE_AXIS_END-22 ;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);

        memset(buffer, 0, 6);
        _float_to_char(upper_w, buffer, 6);
        char_x=BODE_AXIS_WIDTH - 2*MINI_WIDTH, char_y= BODE_AXIS_END-22;
        PrintMiniMini(&char_x, &char_y, buffer, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK, TEXT_MODE_NORMAL);



        // draw gain
        for(int x = 0; x< BODE_AXIS_WIDTH; x++){
            y = BODE_AXIS_HEIGHT-(int)((g[x]-min_g)/(max_g-min_g) * BODE_AXIS_HEIGHT);
            switch (graph_mode) {
            case (DOTTED):
                plot(x+BODE_AXIS_GAP_X,y,COLOR_GREEN);
                break;
            case LINE:
                drawLine((x-1), old_y, x, y, COLOR_GREEN);
                break;
            }
            old_y = y;
        }
        //draw phase
        for (int x = 0; x< BODE_AXIS_WIDTH; x++){
            y = BODE_AXIS_END-(int)((p[x]-min_p)/(max_p-min_p) * BODE_AXIS_HEIGHT);
            switch (graph_mode) {
            case DOTTED:
                plot(x + BODE_AXIS_GAP_X,y,COLOR_GREEN);
                break;
            case LINE:
                drawLine((x-1), old_y2, x, y, COLOR_GREEN);
                break;
            }
            old_y2 = y;
        }

        int key;
        GetKey(&key);
        if (key == KEY_CTRL_EXE || key == KEY_CTRL_EXIT)
            break;
        if (key == KEY_CTRL_RIGHT){
            lower_w *= 10;
            upper_w *= 10;
            step_w *= 10;
        }
        if (key == KEY_CTRL_LEFT){
            lower_w = lower_w / 10;
            upper_w = upper_w / 10;
            step_w = step_w/10;
        }
        

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
}

// maybe not the best solution but it works and seems safe...ish
void str_to_array(char * str, int array_len, double * array[]){
    char * start = str;
    char * end = str;
    for (int i = 0; i<array_len && *end != '\0'; i++){
        *array[i] = strtod(start, &end);
        // Hacky solution to remove a single whitespace or comma thats seperating numbers
        start = end + 1;
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
    double * a[3];
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
                text_write_buffer(first.edit, text_edit);
                a[0] = &first.a; a[1] = &first.b;
                str_to_array(first.edit, 2, a);
                if (first.b/first.a < 0){
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
                _float_to_char(1/first.b, buffer + n-1, 5);
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
                text_write_buffer(second.edit, text_edit);
                a[0] = &second.a; a[1] = &second.b; a[2] = &second.c;
                str_to_array(second.edit, 3, a);
                if (sqr(second.b) < 4* second.a * second.c){
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
                double wn = s21_norm_sqrt(second.c/second.a);
                double zeta = second.b/(2.*s21_norm_sqrt(second.a*second.c));
                double sigma = zeta * wn;
                double wd = wn * s21_norm_sqrt(1-sqr(zeta));
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
                text_write_buffer(bode.edit, text_edit);
                // Bug here
                a[0] = bode.transform;
                str_to_array(bode.edit, 3, a);
                break;
            }
            case 22:
            {
                plot_bode(BODE, DOTTED);
                break;
            }
                
            default:
                break;
        }

        tab = choice / 10;
        
    }
    return 0; 
}

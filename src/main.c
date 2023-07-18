#include "../../../include/fxcg/display.h"
#include "../../../include/fxcg/keyboard.h"
//#include "../../../include/string.h"
#include "../../../include/stdlib.h"
#include "menu.h"
#include "utils.h"
#include "s21_math.h"


// might split bode into gain and phase
static struct menu_tab Tab0 = {"First", 5, {{1, "Numerator"}, {2, "Denominator"}, {3, "Step Response"}, {4, "Bode Plot"},{5, "Characteristics"}}};
static struct menu_tab Tab1 = {"Second", 5, {{11, "Numerator"}, {12, "Denominator"}, {13, "Step Response"}, {14, "Bode Plot"}, {15, "Characteristics"}}};
static struct menu_tab Tab2 = {"Bode", 3, {{21, "Numerator"}, {22, "Denominator"}, {23, "Bode Plot"}}};

static struct menu_page Page0 = {"OPT", KEY_CTRL_OPTN, 3, {&Tab0, &Tab1, &Tab2}};

static struct Tmenu geometry = {1, {&Page0}};

// first order  a/(bs + c)
static struct {char num[256], den[256]; double a,b,c;} first;

// second order a/(b s^2 + c s + d)
static struct {char num[256], den[256]; double a,b,c,d;} second;

// bode plot (a0 )
static struct {char num[256], den[256]; double fnum[256], fden[256];} bode;

enum plot_type{
    FIRST, SECOND, BODE
};

enum plot_mode{
    DOTTED, LINE
};



//static char empty_buffer[256];

//static char * empty_text = "--";

const char * text_num = "--Numerator:";
const char * text_den = "--Denominator:";



void set_default_values(){
    first.a = 1.;
    first.b = 1.;
    first.c = 1.;

    second.a = 1.;
    second.b = 1.;
    second.c = 1.;
    second.d = -1.;
}


void draw_bode_axis(void){
    fillArea(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, COLOR_WHITE);

    for (int y=0; y<LCD_HEIGHT_PX; y++){
        plot(0, y, COLOR_LIGHTSLATEGRAY);
        plot(LCD_WIDTH_PX-1, y, COLOR_LIGHTSLATEGRAY);
    }

    for (int x = 0; x<LCD_WIDTH_PX; x++) {
        plot(x, 0, COLOR_LIGHTSLATEGRAY);
        plot(x, LCD_HEIGHT_PX/2, COLOR_LIGHTSLATEGRAY);
        //plot(x, 3*LCD_HEIGHT_PX/4, COLOR_LIGHTSLATEGRAY);
        plot(x, LCD_HEIGHT_PX-1, COLOR_LIGHTSLATEGRAY);
    }

}

int scaled_value(int start, int length, double min_x, double max_x, double x){
    // map x and y between min and max
    x = min(max(x, min_x), max_x);

    x = (x - min_x)/(max_x-min_x) * length + start;
    return (int)x;
}


void draw_step_axis(double start_t, double end_t, double step_t, double min_e, double max_e, double step_e){
    fillArea(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, COLOR_WHITE);

    // Draw grid lines
    for (double y = 0. ; y<max_e; y += step_e){
        int scaled_y = scaled_value(0, LCD_HEIGHT_PX, 0 , max_e, y);
        for (int x = 1; x<=LCD_WIDTH_PX; x++){
            plot(x, scaled_y, COLOR_BLUE);
        }
    }

    for (double x = start_t ; x<end_t; x += step_t){
        int scaled_x = scaled_value(0, LCD_WIDTH_PX, start_t , end_t, x);
        for (int y = 1; y<=LCD_HEIGHT_PX; y++){
            plot(scaled_x, y, COLOR_BLUE);
        }
    }

    for (int y=0; y<LCD_HEIGHT_PX; y++){
        plot(0, y, COLOR_BLACK);
    }
    for (int x = 0; x<LCD_WIDTH_PX; x++) {
        plot(x, LCD_HEIGHT_PX-1, COLOR_BLACK);
    }
}

double second_bode_gain(double w){
    double a=second.a, b=second.b, c=second.c, d=second.d;
    double num = s21_sqrt(sqr(a*d - a*b* sqr(w)) + sqr(a*c*w));
    double den = sqr(d-b* sqr(w)) + c * sqr(w);
    return num/den;
}
double second_bode_phase(double w){
    double a=second.a, b=second.b, c=second.c, d=second.d;
    return s21_atan(- (a*c*w)/(d*a - a*b* sqr(w)) );
}


double first_bode_gain(double w){
    double a = first.a, b = first.b, c = first.c;
    return s21_sqrt(sqr(a*c) + sqr(a*b*w))/(sqr(c) + sqr(b*w));
}

double first_bode_phase(double w){
    double b = first.b, c = first.c;
    return s21_atan(-(b*w)/(c));
}

double first_step(double t){
    //double a = first.a, b = first.b, c = first.c;
    return  (1-1/s21_exp(0.5 * t));
}

double second_step(double t){
    return 0.;
}
double bode_step(double t){
    return 0.;
}


double calc_e(double t){
    return (t<5)? 0: 1;
}


void plot_step(enum plot_type graph_plot, enum plot_mode graph_mode){
    double start_t = 0.;
    double end_t = 10.;

    int pixel_step =1;

    double e [LCD_WIDTH_PX];
    double t [LCD_WIDTH_PX];
    int axis_width = LCD_WIDTH_PX-1;
    int axis_height = (LCD_HEIGHT_PX)-1;


    for(;;){
        // fill array with graph data
        for (int x = 1; x*pixel_step<=axis_width; x++){
            t[x] = (end_t - start_t) * (double) (x*pixel_step) / (double)axis_width + start_t;
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
        double max_e = 1.2;

        // calculate step_e for grid lines

        draw_step_axis(start_t, end_t, 1., 0, max_e, 0.2);
        
        int old_y=0;
        for(int x = 1; x*pixel_step<=axis_width; x++){
            int y = axis_height-(int)(e[x]/max_e * axis_height);
            switch(graph_mode){
            case DOTTED:
                plot(x,y,COLOR_RED);
                break;
            case LINE:
                drawLine((x-1)*pixel_step, old_y, x*pixel_step, y, COLOR_RED);
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
void plot_bode(enum plot_type graph_plot, enum plot_mode graph_mode){
    double lower_w = 0.01, upper_w = 0.1;

    double w[LCD_WIDTH_PX];
    double g[LCD_WIDTH_PX];
    double p[LCD_WIDTH_PX];

    int pixel_step = 1;

    int axis_width = LCD_WIDTH_PX-2;
    int axis_height = (LCD_HEIGHT_PX/2);

    int y, old_y, old_y2;
    old_y = 0;
    old_y2 = 2*axis_height;
    for(;;){
        draw_bode_axis();

        // draw gain graph (fast)
        
        for (int x = 1; x*pixel_step<=axis_width; x++){
            // incorrect but simple
            w[x] = (upper_w-lower_w) * ((x*pixel_step) / (double)axis_width) + lower_w;
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
        double max_p = M_PI, min_p = -M_PI;

        // draw gain
        for(int x = 1; x*pixel_step<=axis_width; x++){
            y = axis_height-(int)((g[x]-min_g)/(max_g-min_g) * axis_height);
            switch (graph_mode) {
            case (DOTTED):
                plot(x,y,COLOR_GREEN);
                break;
            case LINE:
                drawLine((x-1)*pixel_step, old_y, x*pixel_step, y, COLOR_GREEN);
                break;
            }
            old_y = y;
        }
        //draw phase
        for (int x = 1; x*pixel_step<=axis_width; x++){
            y = 2*axis_height-(int)((p[x]-min_p)/(max_p-min_p) * axis_height);
            switch (graph_mode) {
            case DOTTED:
                plot(x,y,COLOR_GREEN);
                break;
            case LINE:
                drawLine((x-1)*pixel_step, old_y2, x*pixel_step, y, COLOR_GREEN);
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
        }
        if (key == KEY_CTRL_LEFT){
            lower_w = lower_w / 10;
            upper_w = upper_w / 10;
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


void str_to_array(char * str, double * array, int array_len){
    char * start = str;
    char * end = str;
    for (int i = 0; i<array_len && *end != '\0'; i++){
        array[i] = strtod(start, &end);
        start = end;
    }
}

int main(void){

    int choice;
    int tab = 0;
    set_default_values();

    // set cursor for writing text

    for(;;){
        // clear display
        Bdisp_AllClr_VRAM();

        // get option from user
        choice = menu(&geometry, 0, tab);

        switch(choice){
            case 1:
                text_write_buffer(first.num, text_num);
                str_to_array(first.num, &(first.a), 1);
                break;
            case 2:
                text_write_buffer(first.den, text_den);
                str_to_array(first.den, &(first.b), 2);
                break;
            case 3:
                plot_step(FIRST, DOTTED);
                break;
            case 4:
                plot_bode(FIRST, DOTTED);
                break;
            case 5:
                // display characteristics
                break;


            case 11:
                text_write_buffer(second.num, text_num);
                str_to_array(second.num, &(second.a), 1);
                break;
            case 12:
                text_write_buffer(second.den, text_den);
                str_to_array(second.num, &(second.b), 3);
                break;
            case 13:
                plot_step(SECOND, DOTTED);
                break;
            case 14:
                plot_bode(SECOND, DOTTED);
                break;
            case 15:
                // display characteristics
                break;

            case 21:
                text_write_buffer(bode.num, text_num);
                str_to_array(bode.num, bode.fnum, 1);
                break;
            case 22:
                text_write_buffer(bode.den, text_den);
                str_to_array(bode.den, bode.fden, 3);
                break;
            case 23:
                plot_bode(BODE, DOTTED);
                break;
                
            default:
                text_write_buffer(first.num, text_num);
                break;
        }

        tab = choice / 10;
        
    }
    return 0; 
}

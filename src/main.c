#include "../../../include/fxcg/display.h"
#include "../../../include/fxcg/keyboard.h"

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

static char empty_buffer[256];

static char * empty_text = "--";

const char * text_num = "--Numerator:";
const char * text_den = "--Denominator:";


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

void draw_step_axis(void){
    fillArea(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, COLOR_WHITE);
    for (int y=0; y<LCD_HEIGHT_PX; y++){
        plot(0, y, COLOR_LIGHTSLATEGRAY);
    }
    for (int x = 0; x<LCD_WIDTH_PX; x++) {
        plot(x, LCD_HEIGHT_PX-1, COLOR_LIGHTSLATEGRAY);
    }
}


double first_bode_gain(double w){
    double a = .5;
    return s21_pow(1+a*a*w*w,0.5) / (1 + a*a*w*w);
}

double first_bode_phase(double w){
    double a = 0.5;
    return s21_atan(-a*w);
}

double first_step(double t){
    return  (1-1/s21_exp(0.5 * t));
}

void plot_second_step(){

}


double calc_e(double t){
    return (t<5)? 0: 1;
}


void plot_step(){
    double end_t = 10.;
    double max_e = 2;
    int axis_width = LCD_WIDTH_PX-1;
    int axis_height = (LCD_HEIGHT_PX)-1;
    double t, e;
    int x, y, old_y;


    for(;;){
        draw_step_axis();

        old_y = axis_height;       
        for (x = 1; x<=axis_width; x++){
            t = end_t * (double) x / (double)axis_width;
            e = first_step(t);
            y = axis_height-(int)(max(min(e, max_e), 0) * axis_height/max_e);

            drawLine(x-1, old_y, x, y, COLOR_RED);
            old_y = y;
        }

        int key;
        GetKey(&key);
        if (key == KEY_CTRL_EXE || key == KEY_CTRL_EXIT)
            break;
    }

}
void plot_bode(){
    double lower_w = 0.01, upper_w = 0.1, w;
    int axis_width = LCD_WIDTH_PX-2;
    int axis_height = (LCD_HEIGHT_PX/2);

    int y, old_y;
    double max_g = 1., min_g = 0.;

    double max_p = 0, min_p = -M_PI_2;

    for(;;){
        draw_bode_axis();

        // draw gain graph
        old_y = axis_height;
        for (int x = 1; x<=axis_width; x++){
            // incorrect but simple
            w = (upper_w-lower_w) * (x / (double)axis_width) + lower_w;
            double g = first_bode_gain(w);
            y = axis_height-(int)(max(min(g, max_g), min_g)/(max_g-min_g) * axis_height);
            drawLine(x-1, old_y, x, y, COLOR_GREEN);
            old_y = y;
        }

        // draw phase graph
        old_y = 2*axis_height;
        for (int x = 1; x<=axis_width; x++){
            // incorrect but simple
            w = (upper_w-lower_w) * (x / (double)axis_width) + lower_w;
            double p = first_bode_phase(w);
            y = axis_height-(int)(max(min(p, max_p), min_p)/(max_p-min_p) * axis_height);
            drawLine(x-1, old_y, x, y, COLOR_GREEN);
            old_y = y;
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
}

int main(void){

    int choice;
    int tab = 0;

    // set cursor for writing text

    for(;;){
        // clear display
        Bdisp_AllClr_VRAM();

        DisplayMBString((unsigned char*)empty_buffer, 0, 0, 1, 2);
        PrintXY(1,1,empty_text,TEXT_MODE_NORMAL,TEXT_COLOR_BLACK);

        // get option from user
        choice = menu(&geometry, 0, tab);

        switch(choice){
            case 1:
                text_write_buffer(first.num, text_num);
                break;
            case 2:
                text_write_buffer(first.den, text_den);
                break;
            case 3:
                plot_step();
                break;
            case 4:
                plot_bode();
                break;
            case 5:
                // display characteristics
                break;


            case 11:
                text_write_buffer(second.num, text_num);
                break;
            case 12:
                text_write_buffer(second.den, text_den);
                break;
            case 13:
                plot_step();
                break;
            case 14:
                plot_bode();
                break;
            case 15:
                // display characteristics
                break;

            case 21:
                text_write_buffer(bode.num, text_num);
                break;
            case 22:
                text_write_buffer(bode.den, text_den);
                break;
            case 23:
                plot_bode();
                break;
                
            default:
                text_write_buffer(first.num, text_num);
                break;
        }

        tab = choice / 10;
        
    }
    return 0; 
}

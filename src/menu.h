#pragma once

#define MENU_X 6
#define MENU_Y 24
#define MENU_HEIGHT 191
#define MENU_WIDTH (LCD_WIDTH_PX - MENU_X)
#define MENU_HEADER_HEIGHT 27

#define MENU_TAB_INDENT 40
#define MENU_TAB_WIDTH 100
#define MENU_TAB_SPACE 4

#define MENU_COLOR COLOR_LIME


struct menu_item{
    int id;
    char *text;
};

struct menu_tab{
    char *label;
    unsigned char item_count;
    struct menu_item items[];
};

struct menu_page{
    char *label;
    int key;
    unsigned char tab_count;
    struct menu_tab *tabs[];
};

struct Tmenu{
    unsigned char page_count;
    struct menu_page *pages[];
};

int menu(struct Tmenu *mymenu, unsigned char selected_page, unsigned char selected_tab);

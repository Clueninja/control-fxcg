//Created by Balping
//http://www.cemetech.net

#include "../../../include/fxcg/display.h"
#include "../../../include/fxcg/keyboard.h"
#include "../../../include/string.h"

#include "menu.h"
#include "utils.h"



void CopySpriteMasked2bitR(const unsigned char* data, int x, int y, int width, int height, const color_t* palette, unsigned char mask){
    short unsigned int* VRAM = (short unsigned int*) GetVRAMAddress();
    VRAM += (LCD_WIDTH_PX*y + x);
    int offset = 0;
    unsigned char buf;
    unsigned char this;
    int availbits = 0;
    for(int j=y; j<y+height; j++) {
        for(int i=x; i<x+width;  i++) {
            if (!availbits) {
                buf = data[offset++];
                availbits = 8;
            }
            this = ((buf&0xC0)>>6);
            if (this != mask){
                *VRAM = palette[(int)this];
            }
            VRAM++;
            buf<<=2;
            availbits-=2;
        }
        VRAM += (LCD_WIDTH_PX-width);
    }
}

int menu(struct Tmenu *mymenu, unsigned char selected_page, unsigned char selected_tab){
    // set palette
    const color_t tab_palette[4] = {MENU_COLOR, COLOR_BLACK, COLOR_WHITE, MENU_COLOR};

    const unsigned char left_corner[18] = {1, 90, 129, 107, 241, 111, 252, 111, 255, 91, 255, 219, 255, 214, 255, 213, 191, 213};
    const unsigned char right_corner[18] = {169, 84, 63, 165, 79, 254, 83, 255, 229, 255, 249, 95, 255, 85, 255, 213, 111, 245};
    
    unsigned char go=1, page, tab, i, starti, selected_item, mehet, item_count;
    unsigned short tabplus=0;
    int x, y, lwidth, key, retval=0;
    unsigned char cheight = (MENU_HEIGHT-MENU_HEADER_HEIGHT-18)/24;
    unsigned char cwidth = (MENU_WIDTH-16)/18;
    char buff[cwidth+1];
    buff[1] = ':'; buff[cwidth] = 0;

    SaveVRAM_1();

    do{
        fillArea(MENU_X, MENU_Y, 1, MENU_HEIGHT, COLOR_BLACK);
        fillArea(MENU_X+1, MENU_Y+MENU_HEADER_HEIGHT+1, 1, MENU_HEIGHT-MENU_HEADER_HEIGHT-3, COLOR_WHITE);
        fillArea(MENU_X+1, MENU_Y, MENU_TAB_INDENT+4, 1, COLOR_BLACK);
        fillArea(MENU_X, MENU_Y+MENU_HEIGHT-2, MENU_WIDTH, 2, COLOR_BLACK);
        fillArea(MENU_X+MENU_WIDTH-2, MENU_Y+MENU_HEADER_HEIGHT+1, 2, MENU_HEIGHT-MENU_HEADER_HEIGHT-3, COLOR_BLACK);
        fillArea(MENU_X, MENU_Y+MENU_HEADER_HEIGHT, MENU_WIDTH, 1, COLOR_BLACK);
        fillArea(MENU_X+1, MENU_Y+MENU_HEADER_HEIGHT+1, MENU_WIDTH-3, 1, COLOR_WHITE);

        fillArea(MENU_X+2, MENU_Y+MENU_HEADER_HEIGHT+2, MENU_WIDTH-4, 4, MENU_COLOR);
        fillArea(MENU_X+2, MENU_Y+MENU_HEIGHT-6, MENU_WIDTH-4, 4, MENU_COLOR);
        fillArea(MENU_X+2, MENU_Y+MENU_HEADER_HEIGHT+6, 4, MENU_HEIGHT-MENU_HEADER_HEIGHT-12, MENU_COLOR);
        fillArea(MENU_WIDTH-MENU_X+6, MENU_Y+MENU_HEADER_HEIGHT+6, 4, MENU_HEIGHT-MENU_HEADER_HEIGHT-12, MENU_COLOR);

        fillArea(MENU_X+6, MENU_Y+MENU_HEADER_HEIGHT+6, 2, MENU_HEIGHT-MENU_HEADER_HEIGHT-13, COLOR_BLACK);
        fillArea(MENU_WIDTH-MENU_X+4, MENU_Y+MENU_HEADER_HEIGHT+6, 1, MENU_HEIGHT-MENU_HEADER_HEIGHT-13, COLOR_BLACK);
        fillArea(MENU_WIDTH-MENU_X+5, MENU_Y+MENU_HEADER_HEIGHT+6, 1, MENU_HEIGHT-MENU_HEADER_HEIGHT-12, COLOR_WHITE);
        fillArea(MENU_X+8, MENU_Y+MENU_HEADER_HEIGHT+6, MENU_WIDTH-16, 2, COLOR_BLACK);
        fillArea(MENU_X+8, MENU_Y+MENU_HEIGHT-8, MENU_WIDTH-16, 1, COLOR_BLACK);
        fillArea(MENU_X+6, MENU_Y+MENU_HEIGHT-7, MENU_WIDTH-13, 1, COLOR_WHITE);

        // for each tab in selected page
        tabplus=0;
        for(tabplus=0; tabplus<mymenu->pages[selected_page]->tab_count*(MENU_TAB_WIDTH+MENU_TAB_SPACE); tabplus+=MENU_TAB_WIDTH+MENU_TAB_SPACE){
            // draw 2 rectangles using the palette using a mask for some reason 
            CopySpriteMasked2bitR(left_corner, tabplus+MENU_X+MENU_TAB_INDENT, MENU_Y+1, 9, 8, tab_palette, 5*!tabplus);
            CopySpriteMasked2bitR(right_corner, tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-9, MENU_Y+1, 9, 8, tab_palette, 0);

            // draw overlapping rectangles for each tab
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT, MENU_Y+9, 1, MENU_HEADER_HEIGHT-9, COLOR_BLACK);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+1, MENU_Y+9, 1, MENU_HEADER_HEIGHT-9, COLOR_WHITE);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+2, MENU_Y+9, 4, MENU_HEADER_HEIGHT-9, MENU_COLOR);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+6, MENU_Y+9, 2, MENU_HEADER_HEIGHT-9, COLOR_BLACK);

            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-8, MENU_Y+9, 1, MENU_HEADER_HEIGHT-9, COLOR_BLACK);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-7, MENU_Y+9, 1, MENU_HEADER_HEIGHT-9, COLOR_WHITE);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-6, MENU_Y+9, 4, MENU_HEADER_HEIGHT-9, MENU_COLOR);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-2, MENU_Y+9, 2, MENU_HEADER_HEIGHT-9, COLOR_BLACK);

            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+9, MENU_Y+6, MENU_TAB_WIDTH-18, 2, COLOR_BLACK);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+9, MENU_Y+2, MENU_TAB_WIDTH-18, 4, MENU_COLOR);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+9, MENU_Y+1, MENU_TAB_WIDTH-18, 1, COLOR_WHITE);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+9, MENU_Y+8, MENU_TAB_WIDTH-18, 1, COLOR_WHITE);
            fillArea(tabplus+MENU_X+MENU_TAB_INDENT+5, MENU_Y, MENU_TAB_WIDTH-9, 1, COLOR_BLACK);
        }
        // draw the selected tab
        tabplus=selected_tab*(MENU_TAB_WIDTH+MENU_TAB_SPACE);
        fillArea(tabplus+MENU_X+MENU_TAB_INDENT+8, MENU_Y+MENU_HEADER_HEIGHT, MENU_TAB_WIDTH-16, 8, COLOR_WHITE);
        fillArea(tabplus+MENU_X+MENU_TAB_INDENT+6, MENU_Y+MENU_HEADER_HEIGHT+1, 2, 5, COLOR_BLACK);
        plot(tabplus+MENU_X+MENU_TAB_INDENT+1, MENU_Y+MENU_HEADER_HEIGHT, COLOR_WHITE);
        fillArea(tabplus+MENU_X+MENU_TAB_INDENT+2, MENU_Y+MENU_HEADER_HEIGHT, 4, 2, MENU_COLOR);
        fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-8, MENU_Y+MENU_HEADER_HEIGHT+1, 1, 5, COLOR_BLACK);
        fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-7, MENU_Y+MENU_HEADER_HEIGHT, 1, 6, COLOR_WHITE);
        fillArea(tabplus+MENU_X+MENU_TAB_INDENT+MENU_TAB_WIDTH-6, MENU_Y+MENU_HEADER_HEIGHT, 4, 2, MENU_COLOR);


        y=(MENU_HEADER_HEIGHT-27)/2 + 10;
        for(tab=0; tab<mymenu->pages[selected_page]->tab_count; tab++){
            x=MENU_X+MENU_TAB_INDENT+tab*(MENU_TAB_WIDTH+MENU_TAB_SPACE)+11;
            fillArea(x-3, MENU_Y+9, MENU_TAB_WIDTH-16, MENU_HEADER_HEIGHT-9, COLOR_WHITE);
            PrintMini(&x, &y, mymenu->pages[selected_page]->tabs[tab]->label, 0x02, x+MENU_TAB_WIDTH-24, 0,0,COLOR_BLACK, COLOR_WHITE, 1,0);
        }

        x=MENU_X+1; lwidth=x;
        y = MENU_HEADER_HEIGHT/2-6;
        PrintMini(&x, &y, mymenu->pages[selected_page]->label, 0, x+MENU_TAB_INDENT-1, 0,0,COLOR_BLACK, MENU_COLOR, 1,0);
        lwidth = x-lwidth;
        x = max(MENU_X + (MENU_TAB_INDENT-1-lwidth)/2 + 1, MENU_X+1);
        fillArea(MENU_X+1, MENU_Y+1, MENU_TAB_INDENT-1, MENU_HEADER_HEIGHT-1, MENU_COLOR);
        PrintMini(&x, &y, mymenu->pages[selected_page]->label, 0, x+MENU_TAB_INDENT-1, 0,0,COLOR_BLACK, MENU_COLOR, 1,0);
    
        fillArea(MENU_X+8, MENU_Y+MENU_HEADER_HEIGHT+8, MENU_WIDTH-16, MENU_HEIGHT-MENU_HEADER_HEIGHT-16, COLOR_WHITE);


        starti=0; selected_item=0;
        mehet=1;
        item_count = mymenu->pages[selected_page]->tabs[selected_tab]->item_count;
        do{ 
            for(i=starti; i<starti+cheight && i<item_count && i<9; i++){
                // draw number for each item as well as text until it can't fit
                buff[0] = '1' + i;
                memset(buff+2, ' ', cwidth-2);
                memcpy(buff+2, mymenu->pages[selected_page]->tabs[selected_tab]->items[i].text,min(strlen(mymenu->pages[selected_page]->tabs[selected_tab]->items[i].text),cwidth-2));
                // underline selected item
                PrintCXY(MENU_X+8, MENU_Y+MENU_HEADER_HEIGHT-15+(i-starti)*24, buff, i==selected_item, -1, COLOR_BLACK, COLOR_WHITE, 1, 0);
                fillArea(MENU_X+8+cwidth*18, MENU_Y+MENU_HEADER_HEIGHT+9+(i-starti)*24, MENU_WIDTH-16-cwidth*18, 24, i==selected_item ? COLOR_BLACK : COLOR_WHITE);
            }
            // draw scroll arrows
            if(starti>0){
                PrintCXY(MENU_X+MENU_WIDTH-26, MENU_Y+MENU_HEADER_HEIGHT-15, "\xE6\x92", TEXT_MODE_NORMAL, -1, starti==selected_item?COLOR_LIME:COLOR_FUCHSIA, starti==selected_item?COLOR_BLACK:COLOR_WHITE, 1, 0);
            }
            if(item_count>starti+cheight){
                PrintCXY(MENU_X+MENU_WIDTH-26, MENU_Y+MENU_HEADER_HEIGHT-15+(cheight-1)*24, "\xE6\x93", TEXT_MODE_NORMAL, -1, starti+cheight-1==selected_item?COLOR_LIME:COLOR_FUCHSIA, starti+cheight-1==selected_item?COLOR_BLACK:COLOR_WHITE, 1, 0);
            }

            GetKey(&key);
            switch(key){
                case KEY_CTRL_UP:
                    selected_item = (selected_item+item_count-1)%item_count;
                    break;
                case KEY_CTRL_DOWN:
                    selected_item = (selected_item+1)%item_count;
                    break;
                case KEY_CHAR_1:
                case KEY_CHAR_2:
                case KEY_CHAR_3:
                case KEY_CHAR_4:
                case KEY_CHAR_5:
                case KEY_CHAR_6:
                case KEY_CHAR_7:
                case KEY_CHAR_8:
                case KEY_CHAR_9:
                    selected_item = min(key-KEY_CHAR_1, item_count-1);
                    break;
                default:
                    mehet=0;
                    break;
            }

            if(starti > selected_item){starti=selected_item;}
            if(starti+cheight <= selected_item){starti=selected_item-cheight+1;}

        } while(mehet);

        switch(key){
            case KEY_CTRL_RIGHT:
                if(mymenu->pages[selected_page]->tab_count-1 == selected_tab){
                    selected_page = (selected_page+1)%mymenu->page_count;
                    selected_tab = 0;
                }else{
                    selected_tab++;
                }
                break;
            case KEY_CTRL_LEFT:
                if(selected_tab == 0){
                    selected_page = (selected_page+mymenu->page_count-1)%mymenu->page_count;
                    selected_tab = mymenu->pages[selected_page]->tab_count-1;
                }else{
                    selected_tab--;
                }
                break;
            case KEY_CTRL_EXIT:
                go=0;
                retval=-1;
                break;
            case KEY_CTRL_EXE:
                go=0;
                retval = mymenu->pages[selected_page]->tabs[selected_tab]->items[selected_item].id;
            default:
                for(page=0; page<mymenu->page_count; page++){
                    if(mymenu->pages[page]->key == key){
                        selected_page = page;
                        selected_tab = 0;
                    }
                }
                break;
        }


        LoadVRAM_1();

    } while(go);


    return retval;
}
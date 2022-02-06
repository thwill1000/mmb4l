/***********************************************************************************************************************
MMBasic

Draw.h

Supporting header file for Draw.c which does the basic LCD display commands and related I/O in MMBasic.

Copyright 2011 - 2018 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/



/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)


void cmd_gui(void);
void cmd_text(void);
void cmd_pixel(void);
void cmd_circle(void);
void cmd_line(void);
void cmd_box(void);
void cmd_rbox(void);
void cmd_triangle(void);
void fun_pixel(void);
void cmd_scroll(void);
//void fun_getscanline(void);
void cmd_cls(void);
void cmd_font(void);
void cmd_colour(void);
void cmd_refresh(void);

void fun_rgb(void);
void fun_mmhres(void);
void fun_mmvres(void);
void fun_mmcharwidth(void);
void fun_mmcharheight(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
        { "SCROLL",         T_CMD,                      0, cmd_scroll        },
        { "GUI",            T_CMD,                      0, cmd_gui        },
        { "Triangle",       T_CMD,                      0, cmd_triangle        },
        { "Text",           T_CMD,                      0, cmd_text        },
        { "Pixel",          T_CMD,                      0, cmd_pixel        },
        { "Circle",         T_CMD,                      0, cmd_circle        },
        { "Line",           T_CMD,                      0, cmd_line        },
        { "Box",            T_CMD,                      0, cmd_box        },
        { "RBox",           T_CMD,                      0, cmd_rbox        },
        { "CLS",            T_CMD,                      0, cmd_cls        },
        { "Font",           T_CMD,                      0, cmd_font        },
        { "Colour",         T_CMD,                      0, cmd_colour        },
        { "Color",          T_CMD,                      0, cmd_colour        },
        { "Refresh",        T_CMD,                      0, cmd_refresh        },

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

        { "RGB(",           T_FUN | T_INT,                0, fun_rgb                },
        { "MM.HRes",            T_FNA | T_INT,                0, fun_mmhres             },
        { "MM.VRes",            T_FNA | T_INT,                0, fun_mmvres             },
        { "MM.FontWidth",   T_FNA | T_INT,                0, fun_mmcharwidth         },
        { "MM.FontHeight",  T_FNA | T_INT,                0, fun_mmcharheight },

        { "Pixel(",                T_FUN | T_INT,                0, fun_pixel,            },
//        { "GetScanLine(",        T_FUN | T_INT,                0, fun_getscanline,        },

#endif


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
  #ifndef DRAW_H_INCL
    #define DRAW_H_INCL

    extern void GUIPrintString(int x, int y, int font, int jh, int jv, int jo, int fc, int bc, char *str);
    extern void GUIPrintChar(int font, int fc, int bc, char c, int o);

    extern void DrawLine(int x1, int y1, int x2, int y2, int w, int c);
    extern void DrawBox(int x1, int y1, int x2, int y2, int w, int c, int fill);
    extern void DrawRBox(int x1, int y1, int x2, int y2, int radius, int c, int fill);
    extern void DrawCircle(int x, int y, int radius, int w, int c, int fill, float aspect);
    extern void DrawPixel(int x, int y, int c);
    extern void ClearScreen(int c);
    extern void SetFont(int fnt);
    extern void ResetDisplay(void);
    extern int GetFontWidth(int fnt);
    extern int GetFontHeight(int fnt);
    extern int rgb(int r, int g, int b);
    extern void (*DrawRectangle)(int x1, int y1, int x2, int y2, int c);
    extern void (*DrawBitmap)(int x1, int y1, int width, int height, int scale, int fc, int bc, char *bitmap);
    extern void (*ScrollLCD) (int lines);
    extern void (*DrawBuffer)(int x1, int y1, int x2, int y2, char *c);
    extern void (*ReadBuffer)(int x1, int y1, int x2, int y2, char *c);
    extern int low_y, high_y, low_x, high_x;
    extern int GetJustification(char *p, int *jh, int *jv, int *jo);
    extern void display_refresh(void);
    extern void DrawRectangleUser(int x1, int y1, int x2, int y2, int c);
    extern void DrawBitmapUser(int x1, int y1, int width, int height, int scale, int fc, int bc, char *bitmap);

    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill);
    #define RGB(red, green, blue) (unsigned int) (((red & 0b11111111) << 16) | ((green  & 0b11111111) << 8) | (blue & 0b11111111))
    #define swap(a, b) {int t = a; a = b; b = t;}

    #define RGB_BLACK           RGB(0,    0,      0)
    #define RGB_BLUE            RGB(0,    0,      255)
    #define RGB_GREEN           RGB(0,    255,    0)
    #define RGB_CYAN            RGB(0,    255,    255)
    #define RGB_RED             RGB(255,  0,      0)
    #define RGB_MAGENTA         RGB(255,  0,      255)
    #define RGB_YELLOW          RGB(255,  255,    0)
    #define RGB_BROWN           RGB(255,  128,    0)
    #define RGB_GRAY            RGB(128,  128,    128)
    #define RGB_LITEGRAY        RGB(210,  210,    210)
    #define RGB_WHITE           RGB(255,  255,    255)

    #define JUSTIFY_LEFT        0
    #define JUSTIFY_CENTER      1
    #define JUSTIFY_RIGHT       2

    #define JUSTIFY_TOP         0
    #define JUSTIFY_MIDDLE      1
    #define JUSTIFY_BOTTOM      2

    #define ORIENT_NORMAL       0
    #define ORIENT_VERT         1
    #define ORIENT_INVERTED     2
    #define ORIENT_CCW90DEG     3
    #define ORIENT_CW90DEG      4


    extern int gui_font;
    extern int gui_font_width, gui_font_height;

    extern int gui_fcolour;
    extern int gui_bcolour;

    extern int DisplayHRes, DisplayVRes;        // resolution of the display
    extern int HRes, VRes;                      // the programming charteristics of the display

    extern int startline;

    #define FONT_BUILTIN_NBR     7              // the number of built in fonts
    #define FONT_TABLE_SIZE     16              // the total size of the font table (builtin + loadable)
    //extern char *FontTable[];
    extern char *blitbuffptr[MAXBLITBUF];


  #endif
#endif






/***********************************************************************************************************************
MMBasic

Draw.c

Does the basic LCD display commands and drawing in MMBasic.

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


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include <unistd.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

/***************************************************************************/
// define the fonts



    #include "X_8x13_LE.h"
    #include "Misc_12x20_LE.h"
    #include "Hom_16x24_LE.h"
    #include "Hom_16x24_BLD.h"
    #include "Inconsola.h"
    #include "ArialNumFontPlus.h"
    #include "Font_8x6.h"

    char *FontTable[FONT_TABLE_SIZE] = {   (char *)X_8x13_LE,
                                                    (char *)Misc_12x20_LE,
                                                    (char *)Hom_16x24_LE,
                                                    (char *)Hom_16x24_BLD,
                                                    (char *)Inconsola, //24x32
                                                    (char *)ArialNumFontPlus, //32x50
                                                    (char *)F_6x8_LE,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                };



/***************************************************************************/

int gui_font;
int gui_fcolour;
int gui_bcolour;
int startline=0;
int CurrentX, CurrentY;                                             // the current default position for the next char to be written
int DisplayHRes, DisplayVRes;                                       // the physical characteristics of the display
int low_y, high_y, low_x, high_x;

// the MMBasic programming characteristics of the display
// note that HRes == 0 is an indication that a display is not configured
int HRes, VRes;

// pointers to the drawing primitives
void (*DrawRectangle)(int x1, int y1, int x2, int y2, int c) = (void (*)(int , int , int , int , int ))DisplayNotSet;
void (*DrawBitmap)(int x1, int y1, int width, int height, int scale, int fc, int bc, char *bitmap) = (void (*)(int , int , int , int , int , int , int , char *))DisplayNotSet;
void (*ScrollLCD) (int lines) = (void (*)(int ))DisplayNotSet;
void (*DrawBuffer)(int x1, int y1, int x2, int y2, char *c) = (void (*)(int , int , int , int , char * ))DisplayNotSet;
void (*ReadBuffer)(int x1, int y1, int x2, int y2, char *c) = (void (*)(int , int , int , int , char * ))DisplayNotSet;
void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill);



/****************************************************************************************************

 MMBasic commands and functions

****************************************************************************************************/

// these are the GUI commands that are common to the MX170 and MX470 versions
// in the case of the MX170 this function is called directly by MMBasic when the GUI command is used
// in the case of the MX470 it is called by MX470GUI in GUI.c
void cmd_guiMX170(void) {
    char *p;

        if(HRes == 0) error("Display not configured");

    // display a bitmap stored in an integer or string
    if((p = checkstring(cmdline, "BITMAP"))) {
        int x, y, fc, bc, h, w, scale, t, bytes;
        char *s;
        MMFLOAT f;
        int64_t i64;

        getargs(&p, 15, ",");
        if(!(argc & 1) || argc < 5) error("Argument count");

        // set the defaults
        h = 8; w = 8; scale = 1; bytes = 8; fc = gui_fcolour; bc = gui_bcolour;

        x = getinteger(argv[0]);
        y = getinteger(argv[2]);

        // get the type of argument 3 (the bitmap) and its value (integer or string)
        t = T_NOTYPE;
        evaluate(argv[4], &f, &i64, &s, &t, true);
        if(t & T_NBR)
            error("Invalid argument");
        else if(t & T_INT)
            s = (char *)&i64;
        else if(t & T_STR)
            bytes = *s++;

        if(argc > 5 && *argv[6]) w = getint(argv[6], 1, HRes);
        if(argc > 7 && *argv[8]) h = getint(argv[8], 1, VRes);
        if(argc > 9 && *argv[10]) scale = getint(argv[10], 1, 15);
        if(argc > 11 && *argv[12]) fc = getint(argv[12], 0, WHITE);
        if(argc == 15) bc = getint(argv[14], -1, WHITE);
        if(h * w > bytes * 8) error("Not enough data");
        DrawBitmap(x, y, w, h, scale, fc, bc, (char *)s);
        return;
    }

    if((p = checkstring(cmdline, "CALIBRATE"))) {
        int tlx, tly, trx, try, blx, bly, brx, bry, midy;
        char *s;
        if(Option.TOUCH_CS == 0) error("Touch not configured");

        GetCalibration(TARGET_OFFSET, TARGET_OFFSET, &tlx, &tly);
        GetCalibration(HRes - TARGET_OFFSET, TARGET_OFFSET, &trx, &try);
        if(abs(trx - tlx) < CAL_ERROR_MARGIN && abs(tly - try) < CAL_ERROR_MARGIN) error("Touch hardware failure");

        GetCalibration(TARGET_OFFSET, VRes - TARGET_OFFSET, &blx, &bly);
        GetCalibration(HRes - TARGET_OFFSET, VRes - TARGET_OFFSET, &brx, &bry);
        midy = max(max(tly, try), max(bly, bry)) / 2;
        Option.TOUCH_SWAPXY = ((tly < midy && try > midy) || (tly > midy && try < midy));

        if(Option.TOUCH_SWAPXY) {
            swap(tlx, tly);
            swap(trx, try);
            swap(blx, bly);
            swap(brx, bry);
        }

        Option.TOUCH_XSCALE = (float)(HRes - TARGET_OFFSET * 2) / (float)(trx - tlx);
        Option.TOUCH_YSCALE = (float)(VRes - TARGET_OFFSET * 2) / (float)(bly - tly);
        Option.TOUCH_XZERO = ((float)tlx - ((float)TARGET_OFFSET / Option.TOUCH_XSCALE));
        Option.TOUCH_YZERO = ((float)tly - ((float)TARGET_OFFSET / Option.TOUCH_YSCALE));
        SaveOptions();
        brx = (HRes - TARGET_OFFSET) - ((brx - Option.TOUCH_XZERO) * Option.TOUCH_XSCALE);
        bry = (VRes - TARGET_OFFSET) - ((bry - Option.TOUCH_YZERO)*Option.TOUCH_YSCALE);
        if(abs(brx) > CAL_ERROR_MARGIN || abs(bry) > CAL_ERROR_MARGIN) {
            s = "Warning: Inaccurate calibration\r\n";
        } else
            s = "Done. No errors\r\n";

        CurrentX = CurrentY = 0;
        MMPrintString(s);
        strcpy(inpbuf, "Deviation X = "); IntToStr(inpbuf + strlen(inpbuf), brx, 10);
        strcat(inpbuf, ", Y = "); IntToStr(inpbuf + strlen(inpbuf), bry, 10); strcat(inpbuf, " (pixels)\r\n");
        MMPrintString(inpbuf);
        if(!Option.DISPLAY_CONSOLE) {
            GUIPrintString(0, 0, 0x11, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, WHITE, BLACK, s);
            GUIPrintString(0, 36, 0x11, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, WHITE, BLACK, inpbuf);
        }



        return;
    }

    if((p = checkstring(cmdline, "TEST"))) {
        if((checkstring(p, "LCDPANEL"))) {
            int t;
            ClearScreen(gui_bcolour);
            t = ((HRes > VRes) ? HRes : VRes) / 7;
            while(getConsole() < '\r') {
                DrawCircle(rand() % HRes, rand() % VRes, (rand() % t) + t/5, 1, 1, rgb((rand() % 8)*256/8, (rand() % 8)*256/8, (rand() % 8)*256/8), 1);
                bufferconsoleinput() ;
                CheckAbort(1);
            }
            ClearScreen(gui_bcolour);
            return;
        }
        if((checkstring(p, "TOUCH"))) {
            int x, y;
            ClearScreen(gui_bcolour);
            while(getConsole() < '\r') {
                x = GetTouch(GET_X_AXIS);
                y = GetTouch(GET_Y_AXIS);
                if(x != TOUCH_ERROR && y != TOUCH_ERROR) DrawBox(x - 1, y - 1, x + 1, y + 1, 0, WHITE, WHITE);
                bufferconsoleinput() ;
                CheckAbort(1);
            }
            ClearScreen(gui_bcolour);
            return;
        }
    }


    if((p = checkstring(cmdline, "RESET"))) {
        if((checkstring(p, "LCDPANEL"))) {
            if(Option.DISPLAY_TYPE == HDMI) InitDisplayHDMI(false);
            if(Option.DISPLAY_TYPE > SSD_PANEL && Option.DISPLAY_TYPE<= SPI_PANEL) InitDisplaySPI(false);
            if(Option.DISPLAY_TYPE >= SSD1963_4 && Option.DISPLAY_TYPE <= SSD_PANEL)InitDisplaySSD(false);
            if(Option.TOUCH_CS) {
               GetTouchValue(CMD_PENIRQ_ON);                                      // send the controller the command to turn on PenIRQ
               GetTouchAxis(CMD_MEASURE_X);
            }
            return;
        }
    }


    error("Invalid command");
}

// get and decode the justify$ string used in TEXT and GUI CAPTION
// the values are returned via pointers
int GetJustification(char *p, int *jh, int *jv, int *jo) {
    switch(toupper(*p++)) {
        case 'L':   *jh = JUSTIFY_LEFT; break;
        case 'C':   *jh = JUSTIFY_CENTER; break;
        case 'R':   *jh = JUSTIFY_RIGHT; break;
        case  0 :   return true;
        default:    p--;
    }
    skipspace(p);
    switch(toupper(*p++)) {
        case 'T':   *jv = JUSTIFY_TOP; break;
        case 'M':   *jv = JUSTIFY_MIDDLE; break;
        case 'B':   *jv = JUSTIFY_BOTTOM; break;
        case  0 :   return true;
        default:    p--;
    }

    skipspace(p);
    switch(toupper(*p++)) {
        case 'N':   *jo = ORIENT_NORMAL; break;                     // normal
        case 'V':   *jo = ORIENT_VERT; break;                       // vertical text (top to bottom)
        case 'I':   *jo = ORIENT_INVERTED; break;                   // inverted
        case 'U':   *jo = ORIENT_CCW90DEG; break;                   // rotated CCW 90 degrees
        case 'D':   *jo = ORIENT_CW90DEG; break;                    // rotated CW 90 degrees
        case  0 :   return true;
        default:    return false;
    }

    return *p == 0;
}
#define SSD1963_WR_TOGGLE_PIN {gpioWrite_Bits_0_31_Clear(1<<12);gpioWrite_Bits_0_31_Set(1<<12);}
void display_refresh(void){
    int t, i, j;
    uint32_t s1,c1, last_s1=0xFFFFFFFF;
    if(high_y>=low_y){
        if(Option.DISPLAY_TYPE > SSD_PANEL && Option.DISPLAY_TYPE<= SPI_PANEL){
            gofifo
            DefineRegion(low_x, low_y, high_x, high_y, 1);
            PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
            set_cs();
            for(i=low_y; i<=high_y; i++){
                t=(i * HRes + low_x) * 2;
                spiWrite(SPIHandle,&screenbuff[t],(high_x-low_x+1) * 2);
            }
            SpiCsHigh(Option.LCD_CS);                                       //set CS high
            low_y=480; high_y=0; low_x=800; high_x=0;
            gonormal
        } else if(Option.DISPLAY_TYPE<=SSD_PANEL && Option.DISPLAY_TYPE>=SSD1963_4){
            gofifo
            SetAreaSSD1963(low_x, low_y, high_x, high_y);
            WriteSSD1963Command(CMD_WR_MEMSTART);
            for(i=low_y; i<=high_y; i++){
                t=(i * HRes + low_x) * 3;
                for(j=low_x; j<=high_x; j++){
                    s1=screenbuff[t++];
                    if(last_s1 != s1){
                        last_s1=s1;
                        c1 = (s1 ^ 0xff) & 0xff;
                        s1=s1<<19;
                        c1=c1<<19;
                        gpioWrite_Bits_0_31_Set(s1);
                        gpioWrite_Bits_0_31_Clear(c1 | (1<<12));
                    }  else gpioWrite_Bits_0_31_Clear(1<<12);
                    gpioWrite_Bits_0_31_Set(1<<12);
                    s1=screenbuff[t++];
                    if(last_s1 != s1){
                        last_s1=s1;
                        c1 = (s1 ^ 0xff) & 0xff;
                        s1=s1<<19;
                        c1=c1<<19;
                        gpioWrite_Bits_0_31_Set(s1);
                        gpioWrite_Bits_0_31_Clear(c1 | (1<<12));
                    }  else gpioWrite_Bits_0_31_Clear(1<<12);
                    gpioWrite_Bits_0_31_Set(1<<12);
                    s1=screenbuff[t++];
                    if(last_s1 != s1){
                        last_s1=s1;
                        c1 = (s1 ^ 0xff) & 0xff;
                        s1=s1<<19;
                        c1=c1<<19;
                        gpioWrite_Bits_0_31_Set(s1);
                        gpioWrite_Bits_0_31_Clear(c1 | (1<<12));
                    }  else gpioWrite_Bits_0_31_Clear(1<<12);
                    gpioWrite_Bits_0_31_Set(1<<12);
                }
            }
            gonormal
            low_y=480; high_y=0; low_x=800; high_x=0;
        } else if(Option.DISPLAY_TYPE == HDMI){
            int spos=low_y * 4 * HRes;
            int tlen= (high_y-low_y+1)* 4 * HRes;
            lseek(fbfd,spos,SEEK_SET);
            write(fbfd,&screenbuff[spos],tlen);
            low_y=1080; high_y=0; low_x=1920; high_x=0;

        } else error("Display not configured");
    }
}
void cmd_refresh(void){
    if(Option.DISPLAY_TYPE==USER)error("Invalid command for display type USER");
    display_refresh();
}

void cmd_text(void) {
    int x, y, font, scale, fc, bc;
    char *s;
    int jh = 0, jv = 0, jo = 0;

    getargs(&cmdline, 17, ",");                                     // this is a macro and must be the first executable stmt
    if(!(argc & 1) || argc < 5) error("Argument count");
    x = getinteger(argv[0]);
    y = getinteger(argv[2]);
    s = getCstring(argv[4]);

    if(argc > 5 && *argv[6])
        if(!GetJustification(argv[6], &jh, &jv, &jo))
            if(!GetJustification(getCstring(argv[6]), &jh, &jv, &jo))
                error("Justification");;

    font = (gui_font >> 4) + 1; scale = (gui_font & 0b1111); fc = gui_fcolour; bc = gui_bcolour;        // the defaults
        if(argc > 7 && *argv[8]) font = getint(argv[8], 1, FONT_TABLE_SIZE);
    if(FontTable[font - 1] == NULL) error("Invalid font #%", font);
        if(argc > 9 && *argv[10]) scale = getint(argv[10], 1, 15);
        if(argc > 11 && *argv[12]) fc = getint(argv[12], 0, WHITE);
        if(argc ==15) bc = getint(argv[14], -1, WHITE);
    GUIPrintString(x, y, ((font - 1) << 4) | scale, jh, jv, jo, fc, bc, s);
}

void getargaddress(char *p, long long int **ip, long MMFLOAT **fp, int *n){
    char *ptr;
    *fp=NULL;
    *ip=NULL;
    if(!isnamestart(*p)){ //found a literal
        *n=1;
        return;
    }
    ptr = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR );
    if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[0] <= 0){ //simple variable
            *n=1;
            return;
        } else { // array or array element
            if(*n == 0)*n=vartbl[VarIndex].dims[0] + 1 - OptionBase;
            else *n = (vartbl[VarIndex].dims[0] + 1 - OptionBase)< *n ? (vartbl[VarIndex].dims[0] + 1 - OptionBase) : *n;
            skipspace(p);
            do {
                p++;
            } while(isnamechar(*p));
            if(*p == '!') p++;
            if(*p == '(') {
                p++;
                skipspace(p);
                if(*p != ')') { //array element
                    *n=1;
                    return;
                }
            }
        }
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        *fp = (long MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[0] <= 0){
            *n=1;
            return;
        } else {
            if(*n == 0)*n=vartbl[VarIndex].dims[0] + 1 - OptionBase;
            else *n = (vartbl[VarIndex].dims[0] + 1 - OptionBase)< *n ? (vartbl[VarIndex].dims[0] + 1 - OptionBase) : *n;
            skipspace(p);
            do {
                p++;
            } while(isnamechar(*p));
            if(*p == '%') p++;
            if(*p == '(') {
                p++;
                skipspace(p);
                if(*p != ')') { //array element
                    *n=1;
                    return;
                }
            }
        }
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        *ip = (long long int *)ptr;
    } else error("Invalid variable");
}
void cmd_scroll(void){
    getargs(&cmdline, 1, ",");
    int lines=getint(argv[0],-(VRes-1), VRes-1);
    ScrollLCD(lines);
}


void cmd_pixel(void) {
    int x1, y1, c=0, n=0 ,i, nc=0;
    long long int *x1ptr, *y1ptr, *cptr;
    long MMFLOAT *x1fptr, *y1fptr, *cfptr;
    getargs(&cmdline, 5, ",");
    if(!(argc == 3 || argc == 5)) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if(n != 1) getargaddress(argv[2], &y1ptr, &y1fptr, &n);
    if(n==1){ //just a single point
        c = gui_fcolour;                                    // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        if(argc == 5)
            c = getint(argv[4], 0, WHITE);
        else
            c = gui_fcolour;
        DrawPixel(x1, y1, c);
    } else {
        c = gui_fcolour;                                        // setup the defaults
        if(argc == 5){
            getargaddress(argv[4], &cptr, &cfptr, &nc);
            if(nc == 1) c = getint(argv[4], 0, WHITE);
            else if(nc>1) {
                if(nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if(c < 0 || c > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        for(i=0;i<n;i++){
            x1 = (x1fptr == NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr == NULL ? y1ptr[i] : (int)y1fptr[i]);
            if(nc > 1) c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
            DrawPixel(x1, y1, c);
        }
    }
    if(Option.refresh)display_refresh();
}


void cmd_circle(void) {
    int x, y, r, w=0, c=0, f=0, n=0 ,i, nc=0, nw=0, nf=0, na=0;
    long MMFLOAT a;
    long long int *xptr, *yptr, *rptr, *fptr, *wptr, *cptr, *aptr;
    long MMFLOAT *xfptr, *yfptr, *rfptr, *ffptr, *wfptr, *cfptr, *afptr;
    getargs(&cmdline, 13, ",");
    if(!(argc & 1) || argc < 5) error("Argument count");
    getargaddress(argv[0], &xptr, &xfptr, &n);
    if(n != 1) {
        getargaddress(argv[2], &yptr, &yfptr, &n);
        getargaddress(argv[4], &rptr, &rfptr, &n);
    }
    if(n==1){
        w = 1; c = gui_fcolour; f = -1; a = 1;                          // setup the defaults
        x = getinteger(argv[0]);
        y = getinteger(argv[2]);
        r = getinteger(argv[4]);
        if(argc > 5 && *argv[6]) w = getint(argv[6], 0, 100);
        if(argc > 7 && *argv[8]) a = getnumber(argv[8]);
        if(argc > 9 && *argv[10]) c = getint(argv[10], 0, WHITE);
        if(argc > 11) f = getint(argv[12], -1, WHITE);
        DrawCircle(x, y, r, w, c, f, a);
    } else {
        w = 1; c = gui_fcolour; f = -1; a = 1;                          // setup the defaults
        if(argc > 5 && *argv[6]) {
            getargaddress(argv[6], &wptr, &wfptr, &nw);
            if(nw == 1) w = getint(argv[6], 0, 100);
            else if(nw>1) {
                if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
                for(i=0;i<nw;i++){
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
                }
            }
        }
        if(argc > 7 && *argv[8]){
            getargaddress(argv[8], &aptr, &afptr, &na);
            if(na == 1) a = getnumber(argv[8]);
            if(na > 1 && na < n) n=na; //adjust the dimensionality
        }
        if(argc > 9 && *argv[10]){
            getargaddress(argv[10], &cptr, &cfptr, &nc);
            if(nc == 1) c = getint(argv[10], 0, WHITE);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if(c < 0 || c > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        if(argc > 11){
            getargaddress(argv[12], &fptr, &ffptr, &nf);
            if(nf == 1) f = getint(argv[12], -1, WHITE);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                    f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                    if(f < 0 || f > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        for(i=0;i<n;i++){
            x = (xfptr==NULL ? xptr[i] : (int)xfptr[i]);
            y = (yfptr==NULL ? yptr[i] : (int)yfptr[i]);
            r = (rfptr==NULL ? rptr[i] : (int)rfptr[i])-1;
            if(nw > 1) w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
            if(nc > 1) c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
            if(nf > 1) f = (ffptr==NULL ? fptr[i] : (int)ffptr[i]);
            if(na > 1) f = (ffptr==NULL ? (long MMFLOAT)fptr[i] : ffptr[i]);
            DrawCircle(x, y, r, w, c, f, a);
        }
    }
}



void cmd_line(void) {
    int x1, y1, x2, y2, w=0, c=0, n=0 ,i, nc=0, nw=0;
    long long int *x1ptr, *y1ptr, *x2ptr, *y2ptr, *wptr, *cptr;
    long MMFLOAT *x1fptr, *y1fptr, *x2fptr, *y2fptr, *wfptr, *cfptr;
    getargs(&cmdline, 11, ",");
    if(!(argc & 1) || argc < 7) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if(n != 1) {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        getargaddress(argv[4], &x2ptr, &x2fptr, &n);
        getargaddress(argv[6], &y2ptr, &y2fptr, &n);
    }
    if(n==1){
        c = gui_fcolour;  w = 1;                                        // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        x2 = getinteger(argv[4]);
        y2 = getinteger(argv[6]);
        if(argc > 7 && *argv[8]){
            w = getint(argv[8], 1, 100);
        }
        if(argc == 11) c = getint(argv[10], 0, WHITE);
        DrawLine(x1, y1, x2, y2, w, c);
    } else {
        c = gui_fcolour;  w = 1;                                        // setup the defaults
        if(argc > 7 && *argv[8]){
            getargaddress(argv[8], &wptr, &wfptr, &nw);
            if(nw == 1) w = getint(argv[8], 0, 100);
            else if(nw>1) {
                if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
                for(i=0;i<nw;i++){
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
                }
            }
        }
        if(argc == 11){
            getargaddress(argv[10], &cptr, &cfptr, &nc);
            if(nc == 1) c = getint(argv[10], 0, WHITE);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if(c < 0 || c > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        for(i=0;i<n;i++){
            x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
            x2 = (x2fptr==NULL ? x2ptr[i] : (int)x2fptr[i]);
            y2 = (y2fptr==NULL ? y2ptr[i] : (int)y2fptr[i]);
            if(nw > 1) w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
            if(nc > 1) c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
            DrawLine(x1, y1, x2, y2, w, c);
        }
    }
}

void cmd_box(void) {
    int x1, y1, wi, h, w=0, c=0, f=0,  n=0 ,i, nc=0, nw=0, nf=0;
    long long int *x1ptr, *y1ptr, *wiptr, *hptr, *wptr, *cptr, *fptr;
    long MMFLOAT *x1fptr, *y1fptr, *wifptr, *hfptr, *wfptr, *cfptr, *ffptr;
    getargs(&cmdline, 13, ",");
    if(!(argc & 1) || argc < 7) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if(n != 1) {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        getargaddress(argv[4], &wiptr, &wifptr, &n);
        getargaddress(argv[6], &hptr, &hfptr, &n);
    }
    if(n == 1){
        c = gui_fcolour; w = 1; f = -1;                                 // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        wi = getinteger(argv[4]) - 1;
        h = getinteger(argv[6]) - 1;
        if(argc > 7 && *argv[8]) w = getint(argv[8], 0, 100);
        if(argc > 9 && *argv[10]) c = getint(argv[10], 0, WHITE);
        if(argc == 13) f = getint(argv[12], -1, WHITE);
        if(wi >= 0 && h >= 0) DrawBox(x1, y1, x1 + wi, y1 + h, w, c, f);
    } else {
        c = gui_fcolour;  w = 1;                                        // setup the defaults
        if(argc > 7 && *argv[8]){
            getargaddress(argv[8], &wptr, &wfptr, &nw);
            if(nw == 1) w = getint(argv[8], 0, 100);
            else if(nw>1) {
                if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
                for(i=0;i<nw;i++){
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
                }
            }
        }
        if(argc > 9 && *argv[10]) {
            getargaddress(argv[10], &cptr, &cfptr, &nc);
            if(nc == 1) c = getint(argv[10], 0, WHITE);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if(c < 0 || c > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        if(argc == 13){
            getargaddress(argv[12], &fptr, &ffptr, &nf);
            if(nf == 1) c = getint(argv[12], 0, WHITE);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                    f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                    if(f < -1 || f > WHITE) error("% is invalid (valid is % to %)", (int)c, -1, WHITE);
                }
            }
        }
        for(i=0;i<n;i++){
            x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
            wi = (wifptr==NULL ? wiptr[i] : (int)wifptr[i])-1;
            h =  (hfptr==NULL ? hptr[i] : (int)hfptr[i])-1;
            if(nw > 1) w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
            if(nc > 1) c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
            if(nf > 1) f = (ffptr==NULL ? fptr[i] : (int)ffptr[i]);
            if(wi >= 0 && h >= 0) DrawBox(x1, y1, x1 + wi, y1 + h, w, c, f);
        }
    }
}


void cmd_rbox(void) {
    int x1, y1, wi, h, w=0, c=0, f=0,  r=0, n=0 ,i, nc=0, nw=0, nf=0;
    long long int *x1ptr, *y1ptr, *wiptr, *hptr, *wptr, *cptr, *fptr;
    long MMFLOAT *x1fptr, *y1fptr, *wifptr, *hfptr, *wfptr, *cfptr, *ffptr;
    getargs(&cmdline, 13, ",");
    if(!(argc & 1) || argc < 7) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if(n != 1) {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        getargaddress(argv[4], &wiptr, &wifptr, &n);
        getargaddress(argv[6], &hptr, &hfptr, &n);
    }
    if(n == 1){
        c = gui_fcolour; w = 1; f = -1; r = 10;                         // setup the defaults
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        w = getinteger(argv[4]) - 1;
        h = getinteger(argv[6]) - 1;
        if(argc > 7 && *argv[8]) r = getint(argv[8], 0, 100);
        if(argc > 9 && *argv[10]) c = getint(argv[10], 0, WHITE);
        if(argc == 13) f = getint(argv[12], -1, WHITE);
        if(w >= 0 && h >= 0) DrawRBox(x1, y1, x1 + w, y1 + h, r, c, f);
    } else {
        c = gui_fcolour;  w = 1;                                        // setup the defaults
        if(argc > 7 && *argv[8]){
            getargaddress(argv[8], &wptr, &wfptr, &nw);
            if(nw == 1) w = getint(argv[8], 0, 100);
            else if(nw>1) {
                if(nw > 1 && nw < n) n=nw; //adjust the dimensionality
                for(i=0;i<nw;i++){
                    w = (wfptr == NULL ? wptr[i] : (int)wfptr[i]);
                    if(w < 0 || w > 100) error("% is invalid (valid is % to %)", (int)w, 0, 100);
                }
            }
        }
        if(argc > 9 && *argv[10]) {
            getargaddress(argv[10], &cptr, &cfptr, &nc);
            if(nc == 1) c = getint(argv[10], 0, WHITE);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if(c < 0 || c > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        if(argc == 13){
            getargaddress(argv[12], &fptr, &ffptr, &nf);
            if(nf == 1) c = getint(argv[12], 0, WHITE);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                    f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                    if(f < -1 || f > WHITE) error("% is invalid (valid is % to %)", (int)c, -1, WHITE);
                }
            }
        }
        for(i=0;i<n;i++){
            x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
            wi = (wifptr==NULL ? wiptr[i] : (int)wifptr[i])-1;
            h =  (hfptr==NULL ? hptr[i] : (int)hfptr[i])-1;
            if(nw > 1) w = (wfptr==NULL ? wptr[i] : (int)wfptr[i]);
            if(nc > 1) c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
            if(nf > 1) f = (ffptr==NULL ? fptr[i] : (int)ffptr[i]);
            if(wi >= 0 && h >= 0) DrawRBox(x1, y1, x1 + wi, y1 + h, w, c, f);
        }
    }
}


// these three functions were written by Peter Mather (matherp on the Back Shed forum)

// read the contents of a PIXEL out of screen memory
void fun_pixel(void) {
    if((void *)ReadBuffer == (void *)DisplayNotSet) error("Invalid on this display");
    int p;
    int x, y;
        getargs(&ep, 3, ",");
        if(argc != 3) error("Argument count");
    x = getinteger(argv[0]);
    y = getinteger(argv[2]);
    ReadBuffer(x, y, x, y, (char *)&p);
    iret = p & 0xFFFFFF;
    targ = T_INT;
}

void cmd_triangle(void) {                                           // thanks to Peter Mather (matherp on the Back Shed forum)
    int x1, y1, x2, y2, x3, y3, c=0, f=0,  n=0,i, nc=0, nf=0;
    long long int *x3ptr, *y3ptr, *x1ptr, *y1ptr, *x2ptr, *y2ptr, *fptr, *cptr;
    long MMFLOAT *x3fptr, *y3fptr, *x1fptr, *y1fptr, *x2fptr, *y2fptr, *ffptr, *cfptr;
    getargs(&cmdline, 15, ",");
    if(!(argc & 1) || argc < 11) error("Argument count");
    getargaddress(argv[0], &x1ptr, &x1fptr, &n);
    if(n != 1) {
        getargaddress(argv[2], &y1ptr, &y1fptr, &n);
        getargaddress(argv[4], &x2ptr, &x2fptr, &n);
        getargaddress(argv[6], &y2ptr, &y2fptr, &n);
        getargaddress(argv[8], &x3ptr, &x3fptr, &n);
        getargaddress(argv[10], &y3ptr, &y3fptr, &n);
    }
    if(n == 1){
        c = gui_fcolour; f = -1;
        x1 = getinteger(argv[0]);
        y1 = getinteger(argv[2]);
        x2 = getinteger(argv[4]);
        y2 = getinteger(argv[6]);
        x3 = getinteger(argv[8]);
        y3 = getinteger(argv[10]);
        if(argc >= 13 && *argv[12]) c = getint(argv[12], BLACK, WHITE);
        if(argc == 15) f = getint(argv[14], -1, WHITE);
        DrawTriangle(x1, y1, x2, y2, x3, y3, c, f);
    } else {
        c = gui_fcolour; f = -1;
        if(argc >= 13 && *argv[12]) {
            getargaddress(argv[12], &cptr, &cfptr, &nc);
            if(nc == 1) c = getint(argv[10], 0, WHITE);
            else if(nc>1) {
                if(nc > 1 && nc < n) n=nc; //adjust the dimensionality
                for(i=0;i<nc;i++){
                    c = (cfptr == NULL ? cptr[i] : (int)cfptr[i]);
                    if(c < 0 || c > WHITE) error("% is invalid (valid is % to %)", (int)c, 0, WHITE);
                }
            }
        }
        if(argc == 15){
            getargaddress(argv[14], &fptr, &ffptr, &nf);
            if(nf == 1) c = getint(argv[14], -1, WHITE);
            else if(nf>1) {
                if(nf > 1 && nf < n) n=nf; //adjust the dimensionality
                for(i=0;i<nf;i++){
                    f = (ffptr == NULL ? fptr[i] : (int)ffptr[i]);
                    if(f < -1 || f > WHITE) error("% is invalid (valid is % to %)", (int)c, -1, WHITE);
                }
            }
        }
        for(i=0;i<n;i++){
            x1 = (x1fptr==NULL ? x1ptr[i] : (int)x1fptr[i]);
            y1 = (y1fptr==NULL ? y1ptr[i] : (int)y1fptr[i]);
            x2 = (x2fptr==NULL ? x2ptr[i] : (int)x2fptr[i]);
            y2 = (y2fptr==NULL ? y2ptr[i] : (int)y2fptr[i]);
            x3 = (x3fptr==NULL ? x3ptr[i] : (int)x3fptr[i]);
            y3 = (y3fptr==NULL ? y3ptr[i] : (int)y3fptr[i]);
            if(nc > 1) c = (cfptr==NULL ? cptr[i] : (int)cfptr[i]);
            if(nf > 1) f = (ffptr==NULL ? fptr[i] : (int)ffptr[i]);
            DrawTriangle(x1, y1, x2, y2, x3, y3, c, f);
        }
    }
}


void cmd_cls(void) {
    char *tp;
    tp = checkstring(cmdline, "CONSOLE");
    if(tp) {
        clear();
        return;
    }
    HideAllControls();

    skipspace(cmdline);
    if(!(*cmdline == 0 || *cmdline == '\''))
        ClearScreen(getint(cmdline, 0, WHITE));
    else
        ClearScreen(gui_bcolour);
    CurrentX = CurrentY = 0;
}



void fun_rgb(void) {
    getargs(&ep, 5, ",");
    if(argc == 5)
        iret = rgb(getint(argv[0], 0, 255), getint(argv[2], 0, 255), getint(argv[4], 0, 255));
    else if(argc == 1) {
            if(checkstring(argv[0], "WHITE"))        iret = WHITE;
            else if(checkstring(argv[0], "BLACK"))   iret = BLACK;
            else if(checkstring(argv[0], "BLUE"))    iret = BLUE;
            else if(checkstring(argv[0], "GREEN"))   iret = GREEN;
            else if(checkstring(argv[0], "CYAN"))    iret = CYAN;
            else if(checkstring(argv[0], "RED"))     iret = RED;
            else if(checkstring(argv[0], "MAGENTA")) iret = MAGENTA;
            else if(checkstring(argv[0], "YELLOW"))  iret = YELLOW;
            else if(checkstring(argv[0], "BROWN"))   iret = BROWN;
            else if(checkstring(argv[0], "GRAY"))    iret = GRAY;
            else error("Invalid colour: $", argv[0]);
    } else
        error("Syntax");
    targ = T_INT;
}



void fun_mmhres(void) {
    iret = HRes;
    targ = T_INT;
}



void fun_mmvres(void) {
    iret = VRes;
    targ = T_INT;
}



void cmd_font(void) {
    getargs(&cmdline, 3, ",");
    if(argc < 1) error("Argument count");
    if(*argv[0] == '#') ++argv[0];
    if(argc == 3)
            SetFont(((getint(argv[0], 1, FONT_TABLE_SIZE) - 1) << 4) | getint(argv[2], 1, 15));
        else
            SetFont(((getint(argv[0], 1, FONT_TABLE_SIZE) - 1) << 4) | 1);

}



void cmd_colour(void) {
    getargs(&cmdline, 3, ",");
    if(argc < 1) error("Argument count");
    gui_fcolour = getint(argv[0], 0, WHITE);
    if(argc == 3)
        gui_bcolour = getint(argv[2], 0, WHITE);


    last_fcolour = gui_fcolour;
    last_bcolour = gui_bcolour;


}


void fun_mmcharwidth(void) {
    if(HRes == 0) error("Display not configured");
    iret = FontTable[gui_font >> 4][0] * (gui_font & 0b1111);
    targ = T_INT;
}


void fun_mmcharheight(void) {
    if(HRes == 0) error("Display not configured");
    iret = FontTable[gui_font >> 4][1] * (gui_font & 0b1111);
    targ = T_INT;
}






/****************************************************************************************************

 General purpose drawing routines

****************************************************************************************************/


void DrawPixel(int x, int y, int c) {
    DrawRectangle(x, y, x, y, c);
}


void ClearScreen(int c) {
    DrawRectangle(0, 0, HRes, VRes, c);
    if(Option.refresh)display_refresh();
}


/**************************************************************************************************
Draw a line on a the video output
        x1, y1 - the start coordinate
        x2, y2 - the end coordinate
    w - the width of the line (ignored for diagional lines)
        c - the colour to use
***************************************************************************************************/
#define abs( a)     (((a)> 0) ? (a) : -(a))

void DrawLine(int x1, int y1, int x2, int y2, int w, int c) {
    int  dx, dy, sx, sy, err, e2;

    if(y1 == y2) {
        DrawRectangle(x1, y1, x2, y2 + w - 1, c);                   // horiz line
        if(Option.refresh)display_refresh();
        return;
    }
    if(x1 == x2) {
        DrawRectangle(x1, y1, x2 + w - 1, y2, c);                   // vert line
        if(Option.refresh)display_refresh();
        return;
    }

    dx = abs(x2 - x1); sx = x1 < x2 ? 1 : -1;
    dy = -abs(y2 - y1); sy = y1 < y2 ? 1 : -1;
    err = dx + dy;
    while(1) {
        DrawPixel(x1, y1, c);
        e2 = 2 * err;
        if (e2 >= dy) {
            if (x1 == x2) break;
            err += dy; x1 += sx;
        }
        if (e2 <= dx) {
            if (y1 == y2) break;
            err += dx; y1 += sy;
        }
    }
    if(Option.refresh)display_refresh();
}



/**********************************************************************************************
Draw a box
     x1, y1 - the start coordinate
     x2, y2 - the end coordinate
     w      - the width of the sides of the box (can be zero)
     c      - the colour to use for sides of the box
     fill   - the colour to fill the box (-1 for no fill)
***********************************************************************************************/
void DrawBox(int x1, int y1, int x2, int y2, int w, int c, int fill) {
    int t;

    // make sure the coordinates are in the right sequence
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(w > x2 - x1) w = x2 - x1;
    if(w > y2 - y1) w = y2 - y1;

    if(w > 0) {
        w--;
        DrawRectangle(x1, y1, x2, y1 + w, c);                       // Draw the top horiz line
        DrawRectangle(x1, y2 - w, x2, y2, c);                       // Draw the bottom horiz line
        DrawRectangle(x1, y1, x1 + w, y2, c);                       // Draw the left vert line
        DrawRectangle(x2 - w, y1, x2, y2, c);                       // Draw the right vert line
        w++;
    }

    if(fill >= 0)
        DrawRectangle(x1 + w, y1 + w, x2 - w, y2 - w, fill);
    if(Option.refresh)display_refresh();
}



/**********************************************************************************************
Draw a box with rounded corners
     x1, y1 - the start coordinate
     x2, y2 - the end coordinate
     radius - the radius (in pixels) of the arc forming the corners
     c      - the colour to use for sides
     fill   - the colour to fill the box (-1 for no fill)
***********************************************************************************************/
void DrawRBox(int x1, int y1, int x2, int y2, int radius, int c, int fill) {
    int f, ddF_x, ddF_y, xx, yy;

    f = 1 - radius;
    ddF_x = 1;
    ddF_y = -2 * radius;
    xx = 0;
    yy = radius;

    while(xx < yy) {
        if(f >= 0) {
            yy-=1;
            ddF_y += 2;
            f += ddF_y;
        }
        xx+=1;
        ddF_x += 2;
        f += ddF_x  ;
        DrawPixel(x2 + xx - radius, y2 + yy - radius, c);              // Bottom Right Corner
        DrawPixel(x2 + yy - radius, y2 + xx - radius, c);              // ^^^
        DrawPixel(x1 - xx + radius, y2 + yy - radius, c);              // Bottom Left Corner
        DrawPixel(x1 - yy + radius, y2 + xx - radius, c);              // ^^^

        DrawPixel(x2 + xx - radius, y1 - yy + radius, c);              // Top Right Corner
        DrawPixel(x2 + yy - radius, y1 - xx + radius, c);              // ^^^
        DrawPixel(x1 - xx + radius, y1 - yy + radius, c);              // Top Left Corner
        DrawPixel(x1 - yy + radius, y1 - xx + radius, c);              // ^^^
        if(fill >= 0) {
            DrawLine(x2 + xx - radius - 1, y2 + yy - radius, x1 - xx + radius + 1, y2 + yy - radius, 1, fill);
            DrawLine(x2 + yy - radius - 1, y2 + xx - radius, x1 - yy + radius + 1, y2 + xx - radius, 1, fill);
            DrawLine(x2 + xx - radius - 1, y1 - yy + radius, x1 - xx + radius + 1, y1 - yy + radius, 1, fill);
            DrawLine(x2 + yy - radius - 1, y1 - xx + radius, x1 - yy + radius + 1, y1 - xx + radius, 1, fill);
        }
    }
    if(fill >= 0) DrawRectangle(x1 + 1, y1 + radius, x2 - 1, y2 - radius, fill);
    DrawRectangle(x1 + radius - 1, y1, x2 - radius + 1, y1, c);         // top side
    DrawRectangle(x1 + radius - 1, y2,  x2 - radius + 1, y2, c);        // botom side
    DrawRectangle(x1, y1 + radius, x1, y2 - radius, c);                 // left side
    DrawRectangle(x2, y1 + radius, x2, y2 - radius, c);                 // right side
    if(Option.refresh)display_refresh();
}




/***********************************************************************************************
Draw a circle on the video output
        x, y - the center of the circle
        radius - the radius of the circle
    w - width of the line drawing the circle
        c - the colour to use for the circle
        fill - the colour to use for the fill or -1 if no fill
        aspect - the ration of the x and y axis (a float).  1.0 gives a prefect circle
***********************************************************************************************/
void DrawCircle(int x, int y, int radius, int w, int c, int fill, float aspect) {
   int a, b, P;
   int A, B;
   int asp;

   while(w >= 0 && radius > 0) {
       a = 0;
       b = radius;
       P = 1 - radius;
       asp = aspect * (float)(1 << 10);

       do {
         A = (a * asp) >> 10;
         B = (b * asp) >> 10;
         if(w) {
             DrawPixel(A+x, b+y, c);
             DrawPixel(B+x, a+y, c);
             DrawPixel(x-A, b+y, c);
             DrawPixel(x-B, a+y, c);
             DrawPixel(B+x, y-a, c);
             DrawPixel(A+x, y-b, c);
             DrawPixel(x-A, y-b, c);
             DrawPixel(x-B, y-a, c);
         }
         if(fill >= 0 && w == 0) {
             DrawRectangle(x-A, y+b, x+A, y+b, fill);
             DrawRectangle(x-A, y-b, x+A, y-b, fill);
             DrawRectangle(x-B, y+a, x+B, y+a, fill);
             DrawRectangle(x-B, y-a, x+B, y-a, fill);
         }
          if(P < 0)
             P+= 3 + 2*a++;
          else
             P+= 5 + 2*(a++ - b--);

        } while(a <= b);
        w--;
        radius--;
    }
    if(Option.refresh)display_refresh();
}



/**********************************************************************************************
Draw a triangle
    Thanks to Peter Mather (matherp on the Back Shed forum)
     x0, y0 - the first corner
     x1, y1 - the second corner
     x2, y2 - the third corner
     c      - the colour to use for sides of the triangle
     fill   - the colour to fill the triangle (-1 for no fill)
***********************************************************************************************/
void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill) {

    if(fill == -1){
        // draw only the outline
        DrawLine(x0, y0, x1, y1, 1, c);
        DrawLine(x1, y1, x2, y2, 1, c);
        DrawLine(x2, y2, x0, y0, 1, c);
    } else {
        //we are drawing a filled triangle which may also have an outline
        long a, b, y, last;
        long  dx01,  dy01,  dx02,  dy02, dx12,  dy12,  sa, sb;

        if (y0 > y1) {
            swap(y0, y1);
            swap(x0, x1);
        }
        if (y1 > y2) {
            swap(y2, y1);
            swap(x2, x1);
        }
        if (y0 > y1) {
            swap(y0, y1);
            swap(x0, x1);
        }

        // Handle awkward all-on-same-line case as its own thing
        if(y0 == y2) {
            a = x0;
            b = x0;
            if(x1 < a) {
                a = x1;
            } else {
            if(x1 > b) b = x1;
            }
            if(x2 < a) {
                a = x2;
            } else {
                if(x2 > b)  b = x2;
            }
            DrawRectangle(a, y0, a, b-a+y0, fill);
        } else {
            dx01 = x1 - x0;  dy01 = y1 - y0;  dx02 = x2 - x0;
            dy02 = y2 - y0; dx12 = x2 - x1;  dy12 = y2 - y1;
            sa = 0; sb = 0;
            if(y1 == y2) {
                last = y1;                                          //Include y1 scanline
            } else {
                last = y1 - 1;                                      // Skip it
            }
            for (y = y0; y <= last; y++){
                a = x0 + sa / dy01;
                b = x0 + sb / dy02;
                sa = sa + dx01;
                sb = sb + dx02;
                a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                if(a > b)swap(a, b);
                DrawRectangle(a, y, b, y, fill);
            }
            sa = dx12 * (y - y1);
            sb = dx02 * ( y- y0);
            while (y <= y2){
                a = x1 + sa / dy12;
                b = x0 + sb / dy02;
                sa = sa + dx12;
                sb = sb + dx02;
                a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
                if(a > b) swap(a, b);
                DrawRectangle(a, y, b, y, fill);
                y = y + 1;
            }
            // we also need an outline but we do this last to overwrite the edge of the fill area
            DrawLine(x0, y0, x1, y1, 1, c);
            DrawLine(x1, y1, x2, y2, 1, c);
            DrawLine(x2, y2, x0, y0, 1, c);
        }
    }
    if(Option.refresh)display_refresh();
}




/******************************************************************************************
 Print a char on the LCD display
 Any characters not in the font will print as a space.
 The char is printed at the current location defined by CurrentX and CurrentY
*****************************************************************************************/
void GUIPrintChar(int fnt, int fc, int bc, char c, int orientation) {
    char *p, *fp, *np = NULL, *AllocatedMemory = NULL;
    int BitNumber, BitPos, x, y, newx, newy, modx, mody, scale = fnt & 0b1111;
    int height, width;

    // to get the +, - and = chars for font 6 we fudge them by scaling up font 1
    if((fnt & 0xf0) == 0x50 && (c == '-' || c == '+' || c == '=')) {
        fp = (char *)FontTable[0];
        scale = scale * 4;
    } else
        fp = (char *)FontTable[fnt >> 4];

    height = fp[1];
    width = fp[0];
    modx = mody = 0;
    if(orientation > ORIENT_VERT){
        AllocatedMemory = np = GetMemory(width * height);
        if (orientation == ORIENT_INVERTED) {
            modx -= width * scale -1;
            mody -= height * scale -1;
        }
        else if (orientation == ORIENT_CCW90DEG) {
            mody -= width * scale;
        }
        else if (orientation == ORIENT_CW90DEG){
            modx -= height * scale -1;
        }
    }

    if(c >= fp[2] && c < fp[2] + fp[3]) {
        p = fp + 4 + (int)(((c - fp[2]) * height * width) / 8);

        if(orientation > ORIENT_VERT) {                             // non-standard orientation
            if (orientation == ORIENT_INVERTED) {
                for(y = 0; y < height; y++) {
                    newy = height - y - 1;
                    for(x=0; x < width; x++) {
                        newx = width - x - 1;
                        if((p[((y * width) + x)/8] >> (((height * width) - ((y * width) + x) - 1) %8)) & 1) {
                            BitNumber=((newy * width) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            } else if (orientation == ORIENT_CCW90DEG) {
                for(y = 0; y < height; y++) {
                    newx = y;
                    for(x = 0; x < width; x++) {
                        newy = width - x - 1;
                        if((p[((y * width) + x)/8] >> (((height * width) - ((y * width) + x) - 1) %8)) & 1) {
                            BitNumber=((newy * height) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            } else if (orientation == ORIENT_CW90DEG) {
                for(y = 0; y < height; y++) {
                    newx = height - y - 1;
                    for(x=0; x < width; x++) {
                        newy = x;
                        if((p[((y * width) + x)/8] >> (((height * width) - ((y * width) + x) - 1) %8)) & 1) {
                            BitNumber=((newy * height) + newx);
                            BitPos = 128 >> (BitNumber % 8);
                            np[BitNumber / 8] |= BitPos;
                        }
                    }
                }
            }
        }  else np = p;

        if(orientation < ORIENT_CCW90DEG) DrawBitmap(CurrentX + modx, CurrentY + mody, width, height, scale, fc, bc, np);
        else DrawBitmap(CurrentX + modx, CurrentY + mody, height, width, scale, fc, bc, np);
    } else {
        if(orientation < ORIENT_CCW90DEG) DrawRectangle(CurrentX + modx, CurrentY + mody, CurrentX + modx + (width * scale), CurrentY + mody + (height * scale), bc);
        else DrawRectangle(CurrentX + modx, CurrentY + mody, CurrentX + modx + (height * scale), CurrentY + mody + (width * scale), bc);
    }

    // to get the . and degree symbols for font 6 we draw a small circle
    if((fnt & 0xf0) == 0x50) {
        if(orientation > ORIENT_VERT) {
            if(orientation == ORIENT_INVERTED) {
                if(c == '.') DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + 7 * scale, 4 * scale, 0, fc, fc, 1.0);
                if(c == 0x60) DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + (height * scale)- 9 * scale, 6 * scale, 2 * scale, fc, -1, 1.0);
            } else if(orientation == ORIENT_CCW90DEG) {
                if(c == '.') DrawCircle(CurrentX + modx + (height * scale) - 7 * scale, CurrentY + mody + (width * scale)/2, 4 * scale, 0, fc, fc, 1.0);
                if(c == 0x60) DrawCircle(CurrentX + modx + 9 * scale, CurrentY + mody + (width * scale)/2, 6 * scale, 2 * scale, fc, -1, 1.0);
            } else if(orientation == ORIENT_CW90DEG) {
                if(c == '.') DrawCircle(CurrentX + modx + 7 * scale, CurrentY + mody + (width * scale)/2, 4 * scale, 0, fc, fc, 1.0);
                if(c == 0x60) DrawCircle(CurrentX + modx + (height * scale)- 9 * scale, CurrentY + mody + (width * scale)/2, 6 * scale, 2 * scale, fc, -1, 1.0);
            }

        } else {
            if(c == '.') DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + (height * scale) - 7 * scale, 4 * scale, 0, fc, fc, 1.0);
            if(c == 0x60) DrawCircle(CurrentX + modx + (width * scale)/2, CurrentY + mody + 9 * scale, 6 * scale, 2 * scale, fc, -1, 1.0);
        }
    }

    if(orientation == ORIENT_NORMAL) CurrentX += width * scale;
    else if (orientation == ORIENT_VERT) CurrentY += height * scale;
    else if (orientation == ORIENT_INVERTED) CurrentX -= width * scale;
    else if (orientation == ORIENT_CCW90DEG) CurrentY -= width * scale;
    else if (orientation == ORIENT_CW90DEG ) CurrentY += width * scale;
    if(orientation > ORIENT_VERT) FreeMemory(AllocatedMemory);
}


/******************************************************************************************
 Print a string on the LCD display
 The string must be a C string (not an MMBasic string)
 Any characters not in the font will print as a space.
*****************************************************************************************/
void GUIPrintString(int x, int y, int fnt, int jh, int jv, int jo, int fc, int bc, char *str) {
    CurrentX = x;  CurrentY = y;
    if(jo == ORIENT_NORMAL) {
        if(jh == JUSTIFY_CENTER) CurrentX -= (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -= (strlen(str) * GetFontWidth(fnt));
        if(jv == JUSTIFY_MIDDLE) CurrentY -= GetFontHeight(fnt) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= GetFontHeight(fnt);
    }
    else if(jo == ORIENT_VERT) {
        if(jh == JUSTIFY_CENTER) CurrentX -= GetFontWidth(fnt)/ 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -= GetFontWidth(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY -= (strlen(str) * GetFontHeight(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= (strlen(str) * GetFontHeight(fnt));
    }
    else if(jo == ORIENT_INVERTED) {
        if(jh == JUSTIFY_CENTER) CurrentX += (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX += (strlen(str) * GetFontWidth(fnt));
        if(jv == JUSTIFY_MIDDLE) CurrentY += GetFontHeight(fnt) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += GetFontHeight(fnt);
    }
    else if(jo == ORIENT_CCW90DEG) {
        if(jh == JUSTIFY_CENTER) CurrentX -=  GetFontHeight(fnt) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX -=  GetFontHeight(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY += (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY += (strlen(str) * GetFontWidth(fnt));
    }
    else if(jo == ORIENT_CW90DEG) {
        if(jh == JUSTIFY_CENTER) CurrentX += GetFontHeight(fnt) / 2;
        if(jh == JUSTIFY_RIGHT)  CurrentX += GetFontHeight(fnt);
        if(jv == JUSTIFY_MIDDLE) CurrentY -= (strlen(str) * GetFontWidth(fnt)) / 2;
        if(jv == JUSTIFY_BOTTOM) CurrentY -= (strlen(str) * GetFontWidth(fnt));
    }
    while(*str) GUIPrintChar(fnt, fc, bc, *str++, jo);
    if(Option.refresh)display_refresh();
}

/****************************************************************************************************
 ****************************************************************************************************

 Basic drawing primitives for a user defined LCD display driver (ie, OPTION LCDPANEL USER)
 all drawing is done using either DrawRectangleUser() or DrawBitmapUser()

 ****************************************************************************************************
****************************************************************************************************/

// Draw a filled rectangle
// this is the basic drawing primitive used by most drawing routines
//    x1, y1, x2, y2 - the coordinates
//    c - the colour
void DrawRectangleUser(int x1, int y1, int x2, int y2, int c){
    char callstr[256];
    char *nextstmtSaved = nextstmt;
    if(FindSubFun("MM.USER_RECTANGLE", 0) >= 0) {
        strcpy(callstr, "MM.USER_RECTANGLE");
        strcat(callstr, " "); IntToStr(callstr + strlen(callstr), x1, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), y1, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), x2, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), y2, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), c, 10);
        callstr[strlen(callstr)+1] = 0;                             // two NULL chars required to terminate the call
        LocalIndex++;
        ExecuteProgram(callstr);
        LocalIndex--;
        nextstmt = nextstmtSaved;
    } else
        error("MM.USER_RECTANGLE not defined");
}


//Print the bitmap of a char on the video output
//    x, y - the top left of the char
//    width, height - size of the char's bitmap
//    scale - how much to scale the bitmap
//          fc, bc - foreground and background colour
//    bitmap - pointer to the bitmap
void DrawBitmapUser(int x1, int y1, int width, int height, int scale, int fc, int bc, char *bitmap){
    char callstr[256];
    char *nextstmtSaved = nextstmt;
    if(FindSubFun("MM.USER_BITMAP", 0) >= 0) {
        if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
        strcpy(callstr, "MM.USER_BITMAP");
        strcat(callstr, " "); IntToStr(callstr + strlen(callstr), x1, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), y1, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), width, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), height, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), scale, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), fc, 10);
        strcat(callstr, ","); IntToStr(callstr + strlen(callstr), bc, 10);
        strcat(callstr, ",&H"); IntToStr(callstr + strlen(callstr), (unsigned int)bitmap, 16);
        callstr[strlen(callstr)+1] = 0;                             // two NULL chars required to terminate the call
        LocalIndex++;
        ExecuteProgram(callstr);
        LocalIndex--;
        nextstmt = nextstmtSaved;
    } else
        error("MM.USER_BITMAP not defined");
}




/****************************************************************************************************

 General purpose routines

****************************************************************************************************/



int rgb(int r, int g, int b) {
    return RGB(r, g, b);
}


inline int GetFontWidth(int fnt) {
    return FontTable[fnt >> 4][0] * (fnt & 0b1111);
}


inline int GetFontHeight(int fnt) {
    return FontTable[fnt >> 4][1] * (fnt & 0b1111);
}


void SetFont(int fnt) {
    if(FontTable[fnt >> 4] == NULL) error("Invalid font number #%", (fnt >> 4)+1);

    gui_font_width = FontTable[fnt >> 4][0] * (fnt & 0b1111);
    gui_font_height = FontTable[fnt >> 4][1] * (fnt & 0b1111);
    if(Option.DISPLAY_CONSOLE) {
        Option.Height = VRes/gui_font_height;
        Option.Width = HRes/gui_font_width;
    }

    gui_font = fnt;
}

void setscroll4P(int t){
    if((Option.DISPLAY_TYPE == SSD1963_4P)){
        if((Option.DISPLAY_ORIENTATION == RLANDSCAPE) || (Option.DISPLAY_ORIENTATION == PORTRAIT)){
            t=592-t;
            if(t<0)t+=864;
        }
        WriteSSD1963Command(CMD_SET_SCROLL_START);
        WriteDataSSD1963(t >> 8);
        WriteDataSSD1963(t);
    } else error("Function not supported on this display");
}


void ResetDisplay(void) {
    if(!Option.DISPLAY_CONSOLE) {
        SetFont(Option.DefaultFont);
        gui_fcolour = Option.DefaultFC;
        gui_bcolour = Option.DefaultBC;
    }
   ResetGUI();

}


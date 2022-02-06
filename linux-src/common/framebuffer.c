/***********************************************************************************************************************
MMBasic

framebuffer.c

Does the basic LCD display commands and drawing in MMBasic.

Copyright 2018 Peter Mather.  All Rights Reserved.

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
void ScrollHDMI(int lines);
extern void display_refresh(void);
#define drawrect 0x11
#define drawbitmap 0x22
#define drawbuffer 0x33
#define scrollbuffer 0x44
#define executecommand 0x55
static union map
{
    unsigned char bbytes[3000];
    short params[1500];
} message;

void process(uint8_t incoming, int reset){
    static int p=0;
    static int commandsize=0;;
    int x1,y1,x2,y2,fc,bc,width,height,scale, lines ;
    static int started=0;
    uint8_t *bitmap;
    static uint8_t xsum=0;
    if(reset){
        started=0;
        xsum=0;
        p=0;
        commandsize=0;
        return;
    }
    if(started<4){
        commandsize=0;
        if(started<3 && incoming==0xAA){
            started++;
            xsum^=incoming;
            message.bbytes[p++]=incoming;
        } else if(started==3 && (incoming == drawrect || incoming == drawbitmap || incoming == drawbuffer || incoming == scrollbuffer || incoming == executecommand)){
            started++;
            xsum^=incoming;
            message.bbytes[p++]=incoming;
         } else {
            started=0;
            xsum=0;
            p=0;
        }
    } else {
        message.bbytes[p++]=incoming;
        xsum^=incoming;
        if(p==6)commandsize=message.params[2];
        if(commandsize>2990){
            p=0;
            commandsize=0;
            started=0;
            xsum=0;
        }
        if(p==commandsize && commandsize){ //We have a complete command to process
           if(xsum == 0){
                if(message.bbytes[3]==drawrect){
                    x1=message.params[3];
                    y1=message.params[4];
                    x2=message.params[5];
                    y2=message.params[6];
                    fc=message.bbytes[14]<<16 | message.bbytes[15]<<8 | message.bbytes[16];
                    if(x1<HRes && y1<VRes && x2>=0 && y2>=0){
                        DrawRectangle(x1,y1,x2,y2,fc);
                        display_refresh();
                    }
                } else if(message.bbytes[3]==drawbitmap){
                    x1=message.params[3];
                    y1=message.params[4];
                    width=message.params[5];
                    height=message.params[6];
                    scale=message.params[7];
                    fc=(message.bbytes[16]<<16) | (message.bbytes[17]<<8) | (message.bbytes[18]);
                    bc=(message.bbytes[19]<<24) | (message.bbytes[20]<<16) | (message.bbytes[21]<<8) | (message.bbytes[22]);
                    bitmap=&message.bbytes[23];
                    if(x1<HRes && y1<VRes && x1+width*scale>=0 && y1+height*scale>=0){
                        DrawBitmap(x1, y1, width, height, scale, fc, bc, (char *)bitmap);
                        display_refresh();
                    }
                } else if(message.bbytes[3]==drawbuffer){
                    x1=message.params[3];
                    y1=message.params[4];
                    x2=message.params[5];
                    y2=message.params[6];
                    bitmap=&message.bbytes[14];
                    DrawBuffer(x1,y1,x2,y2,(char *)bitmap);
                    display_refresh();
                } else if(message.bbytes[3]==scrollbuffer){
                    DrawRectangle(100,100,100,100,0xff00);display_refresh();
                    lines=message.params[3];
                    ScrollLCD(lines);
                } else if(message.bbytes[3]==executecommand){
                }
            }
            p=0;
            commandsize=0;
            started=0;
            xsum=0;
        }
    }
}
void WR4Check(void) {
    uint32_t j;
    int nibble=0;
    uint8_t incoming;
    int lastlevel=1;
    int level;
    int bits;
    int count=0;
    while(1){
        if(count++ == 0x10000){
            CheckAbort(0);
            bufferconsoleinput();
            count=0;
        }
        bits=gpioRead_Bits_0_31();
        if(bits>>6 == 0){ //reset seen
            gpioWrite(27,1);
            lastlevel=1;
            nibble=0;
            process(incoming,1);
        } else {
            level=(bits>>12) & 1;
            if(lastlevel!=level){
                lastlevel=level;
                if(level==0){ //WR seen
                    gpioWrite(27,0);
                    j=(bits>>19 & 0xF);
                    if(nibble==0){
                        incoming=j;
                        nibble=1;
                    } else {
                        j<<=4;
                        incoming |=j;
                        nibble=0;
                        process(incoming,0);
                    }
                } else {
                    gpioWrite(27,1);
                }
            }
        }
    }
}
void WR8Check(void){
    uint8_t incoming;
    int lastlevel=1;
    int level;
    int bits;
    int count=0;
    while(1){
        if(count++ == 0x10000){
            CheckAbort(0);
            bufferconsoleinput();
            count=0;
        }
        bits=gpioRead_Bits_0_31();
        if(bits>>6 == 0){ //reset seen
            gpioWrite(27,1);
            lastlevel=1;
            process(incoming,1);
        } else {
            level=(bits>>12) & 1;
            if(lastlevel!=level){
                lastlevel=level;
                if(level==0){ //WR seen
                    gpioWrite(27,0);
                    incoming=(bits>>19 & 0xFF);
                    process(incoming,0);
                } else {
                    gpioWrite(27,1);
                }
            }
        }
    }
}
void InitRemote(void){
    if(Option.Remote==0)return;
//    gpioCfgClock(1,Option.clock,0);
//    gpioCfgMemAlloc(2);
//    gpioCfgDMAchannels(4,5);
//    gpioCfgBufferSize(1000);
//    gpioInitialise();
//    gpioSetPad(0,10);
//    Option.DISPLAY_TYPE=HDMI;
//    InitDisplayHDMI(true);
//    gpioSetMode(19, PI_INPUT);
//    gpioSetMode(20, PI_INPUT);
//    gpioSetMode(21, PI_INPUT);
//    gpioSetMode(22, PI_INPUT);
//    gpioSetMode(12, PI_INPUT);
//    gpioSetMode(27, PI_OUTPUT);gpioWrite(27,1);
    SetAndReserve(35, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
    SetAndReserve(38, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
    SetAndReserve(40, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
    SetAndReserve(15, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
    if(Option.Remote == 8){
        SetAndReserve(16, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
        SetAndReserve(18, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
        SetAndReserve(22, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
        SetAndReserve(37, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
    }
    SetAndReserve(32, P_INPUT, 0, EXT_BOOT_RESERVED);                            // config data/command as an output
    SetAndReserve(13, P_OUTPUT, 1, EXT_BOOT_RESERVED);                         // config reset as an output
    SetAndReserve(31, P_INPUT, 0,  EXT_BOOT_RESERVED);                         // config reset as an input
    MMPrintString("Running as remote display - Ctrl-C to exit to Basic\r\n");
    refresh();
    uSec(1000000);
    gpioSetTimerFunc(0, 10, NULL);
    if(Option.Remote == 8) WR8Check();
    else WR4Check();
}

void InitDisplayHDMI(int fullinit){
    int i;
    if(Option.DISPLAY_TYPE!=HDMI)return;

    if(fullinit){
        // Open the file for reading and writing
        fbfd = open("/dev/fb0", O_RDWR);
        if (fbfd == -1) {
            error("Cannot open framebuffer device");
        }

        // Get fixed screen information
        if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
            error("Reading fixed information");
        }

        // Get variable screen information
        if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
            error("Reading variable information");
        }
        HRes = vinfo.xres;
        VRes = vinfo.yres;
        screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
   // setup the pointers to the drawing primitives
        DrawRectangle = DrawRectangleHDMI;
        ScrollLCD = ScrollHDMI;
        DrawBuffer=DrawBufferHDMI;
        DrawBitmap = DrawBitmapHDMI;
        ReadBuffer=ReadBufferHDMI;
    }
    lseek(fbfd,0,SEEK_SET);
    for(i=0;i<HRes*VRes*4;i+=4){
        screenbuff[i]=0x00; //Blue
        screenbuff[i+1]=0x00; //green
        screenbuff[i+2]=0x0;  //red
        screenbuff[i+3]=0xFF; //alpha
    }

//    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    write(fbfd,screenbuff,screensize);

    Option.refresh=1;

    return;
}
void ConfigDisplayHDMI(char*p){
    if(checkstring(p, "HDMI")) Option.DISPLAY_TYPE=HDMI;
}
void DrawRectangleHDMI(int x1, int y1, int x2, int y2, int c){
    int i, j,  t;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0;
    if(x1 >= HRes) x1 = HRes - 1;
    if(x2 < 0) x2 = 0;
    if(x2 >= HRes) x2 = HRes - 1;
    if(y1 < 0) y1 = 0;
    if(y1 >= VRes) y1 = VRes - 1;
    if(y2 < 0) y2 = 0;
    if(y2 >= VRes) y2 = VRes - 1;
    if(y1<low_y)low_y=y1;
    if(y2>high_y)high_y=y2;
    if(x1<low_x)low_x=x1;
    if(x2>high_x)high_x=x2;
    // convert the colours to 565 format
    for(i=y1;i<=y2;i++){
        t=(i * HRes + x1) * 4;
        for(j=x1;j<=x2;j++){
            screenbuff[t++]=c & 0xFF;
            screenbuff[t++]=(c>>8)& 0xFF;
            screenbuff[t++]=(c>>16)& 0xFF;
            screenbuff[t++]=0xFF;
        }
    }

}
void DrawBufferHDMI(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2;
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;
    if(yy1<low_y)low_y=yy1;
    if(yy2>high_y)high_y=yy2;
    if(xx1<low_x)low_x=xx1;
    if(xx2>high_x)high_x=xx2;
    t=0;
    for(y=y1;y<=y2;y++){
        t=(y * HRes + x1) * 4;
        for(x=x1;x<=x2;x++){
            if(x>=0 && x<HRes && y>=0 && y<VRes){
                screenbuff[t++]=*p++;
                screenbuff[t++]=*p++;
                screenbuff[t++]=*p++;
                screenbuff[t++]=0xFF;
            } else {
                t+=4;
                p+=3;
            }
        }
    }
}
void DrawBitmapHDMI(int x1, int y1, int width, int height, int scale, int fc, int bc, char *bitmap){
    int i, j, k, m, n, t, x, y;
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    char *p=0;
    char f[3],b[3];
    // adjust when part of the bitmap is outside the displayable coordinates
    if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= HRes) XEnd = HRes - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= VRes) YEnd = VRes - 1;// the height of the bitmap will extend beyond the bottom margin
    if(bc == -1) {                                                     //special case of overlay text
        i = 0;
        j = width * height * scale * scale * 3;
        p = GetMemory(j);                                              //allocate some temporary memory
        ReadBuffer(XStart, y1, XEnd, YEnd, p);
    }
    f[2]= (fc >> 16) & 0xFF ;
    f[1] = (fc >>  8) & 0xFF ;
    f[0] = (fc & 0xFF );
    b[2]= (bc >> 16) & 0xFF ;
    b[1] = (bc >>  8) & 0xFF ;
    b[0] = (bc & 0xFF );

    if(y1<low_y)low_y=y1;
    if(YEnd>high_y)high_y=YEnd;
    if(XStart<low_x)low_x=XStart;
    if(XEnd>high_x)high_x=XEnd;
    // switch to SPI enhanced mode for the bulk transfer
    t = n = 0;
    for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
        for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
            y=vertCoord;
            if(vertCoord++ < 0) continue;                           // we are above the top of the screen
            if(vertCoord > VRes) {                                  // we have extended beyond the bottom of the screen
                if(p != NULL) FreeMemory(p);
                return;
            }
            horizCoord = x1;
            for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                    x=horizCoord;
                    if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                    if(horizCoord > HRes) continue;                 // we are beyond the right margin
                                        t=(y * HRes + x) * 4;
                    if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        screenbuff[t++]=f[0];
                        screenbuff[t++]=f[1];
                        screenbuff[t++]=f[2];
                        screenbuff[t++]=0xFF;
                     } else {
                        if(bc == -1){
                            screenbuff[t++] = p[n];
                            screenbuff[t++] = p[n+1];
                            screenbuff[t++] = p[n+2];
                            screenbuff[t++]=0xFF;
                        } else {
                            screenbuff[t++]=b[0];
                            screenbuff[t++]=b[1];
                            screenbuff[t++]=b[2];
                            screenbuff[t++]=0xFF;
                        }
                    }
                    n += 3;
                }
            }
        }
    }
    if(p != NULL) FreeMemory(p);

}

void ReadBufferHDMI(int x1, int y1, int x2, int y2, char* p){
    int x,y,t;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    for(y=y1;y<=y2;y++){
        t=(y * HRes + x1) * 4;
        for(x=x1;x<=x2;x++){
            if(x>=0 && x<HRes && y>=0 && y<VRes){
                *p++ = screenbuff[t];
                *p++ = screenbuff[t+1];
                *p++ = screenbuff[t+2];
                t+=4;
            } else {
                t+=4;
                *p++=0;
                *p++=0;
                *p++=0;
            }
        }
    }
}

void ScrollHDMI(int lines){
    char *s,*d;
    int y, yy;
    if(lines>0){
        for(y=0;y<VRes-lines;y++){
            yy=y+lines;
            d=(y * HRes) * 4 + screenbuff;
            s=(yy * HRes) * 4 + screenbuff;
            memcpy(d, s, HRes * 4);
        }
        DrawRectangle(0, VRes-lines, HRes - 1, VRes - 1, gui_bcolour); // erase the line to be scrolled off
        low_y=0; high_y=VRes-1; low_x=0; high_x=HRes-1;
        display_refresh();
    } else if(lines<0){
            lines=-lines;
            for(y=VRes-1;y>=lines;y--){
            yy=y-lines;
            d=(y * HRes) * 4 + screenbuff;
            s=(yy * HRes) * 4 + screenbuff;
            memcpy(d, s, HRes * 4);
        }
        DrawRectangle(0, 0, HRes - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
        low_y=0; high_y=VRes-1; low_x=0; high_x=HRes-1;
        display_refresh();
    }
}
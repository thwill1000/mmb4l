/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   framebuffer.h
 * Author: Peter
 *
 * Created on 29 October 2017, 16:51
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ConfigDisplayHDMI(char *p);
extern void InitDisplayHDMI(int fullinit);
extern void DrawRectangleHDMI(int x1, int y1, int x2, int y2, int c);
extern void DrawBitmapHDMI(int x1, int y1, int width, int height, int scale, int fc, int bc, char *bitmap);
extern void DrawBufferHDMI(int x1, int y1, int x2, int y2, char* p);
extern void ReadBufferHDMI(int x1, int y1, int x2, int y2, char* p);
extern void DefineRegionHDMI(int xstart, int ystart, int xend, int yend, int rw);
extern long int screensize;
extern int fbfd;

#ifdef __cplusplus
}
#endif

#endif /* FRAMEBUFFER_H */


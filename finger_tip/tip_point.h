#ifndef _TIP_POINT_H
#define _TIP_POINT_H

#define WIDTH 640
#define HEIGHT 480

/* 函数部分 */
#ifdef __cplusplus
extern "C" {
#endif
 
    int tip_posi(unsigned char *fb, int len, int *x, int *y);

#ifdef __cplusplus
}
#endif

#endif
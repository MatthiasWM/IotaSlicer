/* Copyright (C) 2001-2017 Peter Selinger.
 This file is part of Potrace. It is free software and it is covered
 by the GNU General Public License. See the file COPYING for details. */

/* A simple and self-contained demo of the potracelib API */

#include "IAPotrace.h"

#include "Iota.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"
#include "printer/IAPrinter.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>

#include "potracelib.h"
#include "bitmap.h"


/* ---------------------------------------------------------------------- */
/* auxiliary bitmap functions */

///* macros for writing individual bitmap pixels */
//#define BM_WORDSIZE ((int)sizeof(potrace_word))
//#define BM_WORDBITS (8*BM_WORDSIZE)
//#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
//#define bm_scanline(bm, y) ((bm)->map + (y)*(bm)->dy)
//#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
//#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
//#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
//#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
//#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
//#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
//#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
//#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)
//
///* return new un-initialized bitmap. NULL with errno on error */
//static potrace_bitmap_t *bm_new(int w, int h) {
//    potrace_bitmap_t *bm;
//    int dy = (w + BM_WORDBITS - 1) / BM_WORDBITS;
//
//    bm = (potrace_bitmap_t *) malloc(sizeof(potrace_bitmap_t));
//    if (!bm) {
//        return NULL;
//    }
//    bm->w = w;
//    bm->h = h;
//    bm->dy = dy;
//    bm->map = (potrace_word *) calloc(h, dy * BM_WORDSIZE);
//    if (!bm->map) {
//        free(bm);
//        return NULL;
//    }
//    return bm;
//}
//
///* free a bitmap */
//static void bm_free(potrace_bitmap_t *bm) {
//    if (bm != NULL) {
//        free(bm->map);
//    }
//    free(bm);
//}

void bezier(IAToolpath *toolpath,
            double x1, double y1,
            double x2, double y2,
            double x3, double y3,
            double x4, double y4);


/**
 * Trace the given framebuffer and store the result as a toolpath at layer z.
 *
 * \todo It may be useful to choose a component, r, g, b, or a, and a threshold
 * \todo Conversion to bitmap is expensive. Can't we rewrite that to use bytes?
 * \todo Not handling holes, not handling hierarchies of loops
 * \todo don't render noise specs
 *       http://potrace.sourceforge.net/potracelib.pdf
 */
int potrace(IAFramebuffer *framebuffer, IAToolpathList *toolpath, double z)
{
    int width = framebuffer->width();
    int height = framebuffer->height();

    IAVector3d &printbed = Iota.pCurrentPrinter->pPrintVolume;
    double xScl = printbed.x()/width;
    double yScl = printbed.y()/height;

    int x, y, i;
    potrace_bitmap_t *bm;
    potrace_param_t *param;
    potrace_path_t *p;
    potrace_state_t *st;
    int n, *tag;
    potrace_dpoint_t (*c)[3];

    /* create a bitmap */
    if (framebuffer->pBitmap) {
        bm = bm_dup(framebuffer->pBitmap);
    } else {
        const uint8_t *px = framebuffer->getRawImageRGB();
        bm = bm_new(width, height);
        if (!bm) {
            fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno));
            ::free((void*)px);
            return 1;
        }

        /* fill the bitmap with some pattern */
        for (y=0; y<height; y++) {
            for (x=0; x<width; x++) {
                unsigned char r = px[ (x+width*y)*3 ];
                BM_PUT(bm, x, y, r>128 ? 1 : 0);
            }
        }
        ::free((void*)px);
    }

    /* set tracing parameters, starting from defaults */
    param = potrace_param_default();
    if (!param) {
        fprintf(stderr, "Error allocating parameters: %s\n", strerror(errno));
        return 1;
    }

    /* area of largest path to be ignored */
    param->turdsize = 20;
    /* resolves ambiguous turns in path decomposition */
    param->turnpolicy = POTRACE_TURNPOLICY_MINORITY;
//#define POTRACE_TURNPOLICY_BLACK 0
//#define POTRACE_TURNPOLICY_WHITE 1
//#define POTRACE_TURNPOLICY_LEFT 2
//#define POTRACE_TURNPOLICY_RIGHT 3
//#define POTRACE_TURNPOLICY_MINORITY 4
//#define POTRACE_TURNPOLICY_MAJORITY 5
//#define POTRACE_TURNPOLICY_RANDOM 6
    /* corner threshold */
    param->alphamax = 1.0;
    /* use curve optimization? */
    param->opticurve = 1;
    /* curve optimization tolerance */
    param->opttolerance = 0.2;

    /* trace the bitmap */
    st = potrace_trace(param, bm);
    if (!st || st->status != POTRACE_STATUS_OK) {
        fprintf(stderr, "Error tracing bitmap: %s\n", strerror(errno));
        return 1;
    }
    bm_free(bm);

    IAToolpathLoop *toolpathLoop = nullptr;
    /* draw each curve */
    p = st->plist;
    while (p != NULL) {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        if (!toolpathLoop) {
            toolpathLoop = new IAToolpathLoop(z);
            toolpathLoop->startPath(c[n-1][2].x*xScl, c[n-1][2].y*yScl);
        } else {
            toolpathLoop->continuePath(c[n-1][2].x*xScl, c[n-1][2].y*yScl);
        }
        for (i=0; i<n; i++) {
            int j;
            switch (tag[i]) {
                case POTRACE_CORNER:
                    toolpathLoop->continuePath(c[i][1].x*xScl, c[i][1].y*yScl);
                    toolpathLoop->continuePath(c[i][2].x*xScl, c[i][2].y*yScl);
                    break;
                case POTRACE_CURVETO:
#if 0
                    toolpathLoop->continuePath(c[i][0].x*xScl, c[i][0].y*yScl);
                    toolpathLoop->continuePath(c[i][1].x*xScl, c[i][1].y*yScl);
                    toolpathLoop->continuePath(c[i][2].x*xScl, c[i][2].y*yScl);
#else
                    j = i ? i-1 : n-1;
                    bezier(toolpathLoop,
                           c[j][2].x*xScl, c[j][2].y*yScl,
                           c[i][0].x*xScl, c[i][0].y*yScl,
                           c[i][1].x*xScl, c[i][1].y*yScl,
                           c[i][2].x*xScl, c[i][2].y*yScl);
#endif
                    break;
                default:
                    printf("potrace: unknown tag %d\n", tag[i]);
            }
        }
        /* at the end of a group of a positive path and its negative
         children, fill. */
        //if (p->next == NULL || p->next->sign == '+') {
            // NULL-> close path and quit layer?
            // + -> rapid move to next shape
            toolpathLoop->closePath();
            toolpath->add(toolpathLoop, 0, 0, 0);
            toolpathLoop = nullptr;
        //}
        p = p->next;
    }

    potrace_state_free(st);
    potrace_param_free(param);

    return 0;
}


static const double m_distance_tolerance = 0.01;

// http://www.antigrain.com/research/adaptive_bezier/index.html
void recursive_bezier(IAToolpath *tp,
                      double x1, double y1,
                      double x2, double y2,
                      double x3, double y3,
                      double x4, double y4)
{
    // Calculate all the mid-points of the line segments
    //----------------------
    double x12   = (x1 + x2) / 2;
    double y12   = (y1 + y2) / 2;
    double x23   = (x2 + x3) / 2;
    double y23   = (y2 + y3) / 2;
    double x34   = (x3 + x4) / 2;
    double y34   = (y3 + y4) / 2;
    double x123  = (x12 + x23) / 2;
    double y123  = (y12 + y23) / 2;
    double x234  = (x23 + x34) / 2;
    double y234  = (y23 + y34) / 2;
    double x1234 = (x123 + x234) / 2;
    double y1234 = (y123 + y234) / 2;

    // Try to approximate the full cubic curve by a single straight line
    //------------------
    double dx = x4-x1;
    double dy = y4-y1;

    double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
    double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));

    if((d2 + d3)*(d2 + d3) < m_distance_tolerance * (dx*dx + dy*dy))
    {
        tp->continuePath(x1234, y1234);
        return;
    }

    // Continue subdivision
    //----------------------
    recursive_bezier(tp, x1, y1, x12, y12, x123, y123, x1234, y1234);
    recursive_bezier(tp, x1234, y1234, x234, y234, x34, y34, x4, y4);
}


void bezier(IAToolpath *tp,
            double x1, double y1,
            double x2, double y2,
            double x3, double y3,
            double x4, double y4)
{
    //add_point(x1, y1);
    recursive_bezier(tp, x1, y1, x2, y2, x3, y3, x4, y4);
    tp->continuePath(x4, y4);
}

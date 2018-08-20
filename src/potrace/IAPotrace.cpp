/* Copyright (C) 2001-2017 Peter Selinger.
 This file is part of Potrace. It is free software and it is covered
 by the GNU General Public License. See the file COPYING for details. */

/* A simple and self-contained demo of the potracelib API */

#include "IAPotrace.h"

#include "../Iota.h"
#include "../toolpath/IAToolpath.h"
#include "../opengl/IAFramebuffer.h"
#include "../printer/IAPrinter.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "potracelib.h"


/* ---------------------------------------------------------------------- */
/* auxiliary bitmap functions */

/* macros for writing individual bitmap pixels */
#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8*BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
#define bm_scanline(bm, y) ((bm)->map + (y)*(bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)

/* return new un-initialized bitmap. NULL with errno on error */
static potrace_bitmap_t *bm_new(int w, int h) {
    potrace_bitmap_t *bm;
    int dy = (w + BM_WORDBITS - 1) / BM_WORDBITS;

    bm = (potrace_bitmap_t *) malloc(sizeof(potrace_bitmap_t));
    if (!bm) {
        return NULL;
    }
    bm->w = w;
    bm->h = h;
    bm->dy = dy;
    bm->map = (potrace_word *) calloc(h, dy * BM_WORDSIZE);
    if (!bm->map) {
        free(bm);
        return NULL;
    }
    return bm;
}

/* free a bitmap */
static void bm_free(potrace_bitmap_t *bm) {
    if (bm != NULL) {
        free(bm->map);
    }
    free(bm);
}



/**
 * Trace the give framebuffer and store the result as a toolpath at layer z.
 *
 * \todo It may be useful to choose a component, r, g, b, or a, and a threshold
 * \todo Conversion to bitmap is expensive. Can't we rewrite that to use bytes?
 */
 int potrace(IAFramebuffer *framebuffer, IAToolpath *toolpath, double z)
{
    auto bitmap = framebuffer->makeIntoBitmap();
    const unsigned char *px = bitmap.get();

    int width = framebuffer->pWidth;
    int height = framebuffer->pHeight;

    IAVector3d &printbed = Iota.gPrinter.pBuildVolume;
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
    bm = bm_new(width, height);
    if (!bm) {
        fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno));
        return 1;
    }

    /* fill the bitmap with some pattern */
    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            unsigned char r = px[ (x+256*y)*3 ];
            BM_PUT(bm, x, y, r>128 ? 1 : 0);
        }
    }

    /* set tracing parameters, starting from defaults */
    param = potrace_param_default();
    if (!param) {
        fprintf(stderr, "Error allocating parameters: %s\n", strerror(errno));
        return 1;
    }
    param->turdsize = 0;

    /* trace the bitmap */
    st = potrace_trace(param, bm);
    if (!st || st->status != POTRACE_STATUS_OK) {
        fprintf(stderr, "Error tracing bitmap: %s\n", strerror(errno));
        return 1;
    }
    bm_free(bm);

    char start = 1;
    /* draw each curve */
    p = st->plist;
    while (p != NULL) {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        if (start) {
            toolpath->startPath(c[n-1][2].x*xScl, c[n-1][2].y*yScl, z);
            start = 0;
        } else {
            toolpath->continuePath(c[n-1][2].x*xScl, c[n-1][2].y*yScl, z);
        }
        for (i=0; i<n; i++) {
            switch (tag[i]) {
                case POTRACE_CORNER:
                    toolpath->continuePath(c[i][1].x*xScl, c[i][1].y*yScl, z);
                    toolpath->continuePath(c[i][2].x*xScl, c[i][2].y*yScl, z);
                    break;
                case POTRACE_CURVETO:
                    toolpath->continuePath(c[i][0].x*xScl, c[i][0].y*yScl, z);
                    toolpath->continuePath(c[i][1].x*xScl, c[i][1].y*yScl, z);
                    toolpath->continuePath(c[i][2].x*xScl, c[i][2].y*yScl, z);
                    // FIXME: make this into a nice curve!
                    break;
            }
        }
        /* at the end of a group of a positive path and its negative
         children, fill. */
        if (p->next == NULL || p->next->sign == '+') {
            // TODO: NULL-> close path and quit layer?
            // TODO: + -> rapid move to next shape
            toolpath->closePath();
            start = 1;
        }
        p = p->next;
    }

    potrace_state_free(st);
    potrace_param_free(param);

    return 0;
}

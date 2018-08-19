/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* A simple and self-contained demo of the potracelib API */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "potracelib.h"

void startPath(double x, double y);
void continuePath(double x, double y);
void closePath(void);


#define WIDTH 256
#define HEIGHT 256

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

/* ---------------------------------------------------------------------- */
/* demo */

int potrace_main(const char *filename, unsigned char *px) {
  int x, y, i;
  potrace_bitmap_t *bm;
  potrace_param_t *param;
  potrace_path_t *p;
  potrace_state_t *st;
  int n, *tag;
  potrace_dpoint_t (*c)[3];

  /* create a bitmap */
  bm = bm_new(WIDTH, HEIGHT);
  if (!bm) {
    fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno)); 
    return 1;
  }

  /* fill the bitmap with some pattern */
  for (y=0; y<HEIGHT; y++) {
    for (x=0; x<WIDTH; x++) {
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

#if 0
    FILE *f = fopen(filename, "wb");

  /* output vector data, e.g. as a rudimentary EPS file */
  fprintf(f, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(f, "%%%%BoundingBox: 0 0 %d %d\n", WIDTH, HEIGHT);
  fprintf(f, "gsave\n");

  /* draw each curve */
  p = st->plist;
  while (p != NULL) {
    n = p->curve.n;
    tag = p->curve.tag;
    c = p->curve.c;
    fprintf(f, "%f %f moveto\n", c[n-1][2].x, c[n-1][2].y);
    for (i=0; i<n; i++) {
      switch (tag[i]) {
      case POTRACE_CORNER:
	fprintf(f, "%f %f lineto\n", c[i][1].x, c[i][1].y);
	fprintf(f, "%f %f lineto\n", c[i][2].x, c[i][2].y);
	break;
      case POTRACE_CURVETO:
	fprintf(f, "%f %f %f %f %f %f curveto\n", 
	       c[i][0].x, c[i][0].y,
	       c[i][1].x, c[i][1].y,
	       c[i][2].x, c[i][2].y);
	break;
      }
    }
    /* at the end of a group of a positive path and its negative
       children, fill. */
    if (p->next == NULL || p->next->sign == '+') {
      fprintf(f, "0 setgray fill\n");
    }
    p = p->next;
  }
  fprintf(f, "grestore\n");
  fprintf(f, "%%EOF\n");

    fclose(f);
#elif 1
    char start = 1;
    /* draw each curve */
    p = st->plist;
    while (p != NULL) {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        if (start) {
            startPath(c[n-1][2].x/2, c[n-1][2].y/2);
            start = 0;
        } else {
            continuePath(c[n-1][2].x/2, c[n-1][2].y/2);
        }
        for (i=0; i<n; i++) {
            switch (tag[i]) {
                case POTRACE_CORNER:
                    continuePath(c[i][1].x/2, c[i][1].y/2);
                    continuePath(c[i][2].x/2, c[i][2].y/2);
                    break;
                case POTRACE_CURVETO:
                    //                    fprintf(f, "%f %f %f %f %f %f curveto\n",
                    //                            c[i][0].x, c[i][0].y,
                    //                            c[i][1].x, c[i][1].y,
                    //                            c[i][2].x, c[i][2].y);
                    continuePath(c[i][0].x/2, c[i][0].y/2);
                    continuePath(c[i][1].x/2, c[i][1].y/2);
                    continuePath(c[i][2].x/2, c[i][2].y/2);
                    break;
            }
        }
        /* at the end of a group of a positive path and its negative
         children, fill. */
        if (p->next == NULL || p->next->sign == '+') {
            closePath();
            start = 1;
        }
        p = p->next;
    }
#else
    FILE *f = fopen(filename, "wb");

    /* output vector data, e.g. as a rudimentary EPS file */
    fprintf(f, "; G-Code generated by Iota Slicer\n");
    fprintf(f, "G1 X0 Y0 Z0\n");

    /* draw each curve */
    p = st->plist;
    while (p != NULL) {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        fprintf(f, "%f %f moveto\n", c[n-1][2].x, c[n-1][2].y);
        for (i=0; i<n; i++) {
            switch (tag[i]) {
                case POTRACE_CORNER:
                    fprintf(f, "G1 X%f Y%f\n", c[i][1].x, c[i][1].y);
                    fprintf(f, "G1 X%f Y%f\n", c[i][2].x, c[i][2].y);
                    break;
                case POTRACE_CURVETO:
//                    fprintf(f, "%f %f %f %f %f %f curveto\n",
//                            c[i][0].x, c[i][0].y,
//                            c[i][1].x, c[i][1].y,
//                            c[i][2].x, c[i][2].y);
                    fprintf(f, "G1 X%f Y%f\n", c[i][0].x, c[i][0].y);
                    fprintf(f, "G1 X%f Y%f\n", c[i][1].x, c[i][1].y);
                    fprintf(f, "G1 X%f Y%f\n", c[i][2].x, c[i][2].y);
                    break;
            }
        }
        /* at the end of a group of a positive path and its negative
         children, fill. */
        if (p->next == NULL || p->next->sign == '+') {
            // TODO: close loop, the rapid move to the next loop
//            fprintf(f, "0 setgray fill\n");
        }
        p = p->next;
    }
    fprintf(f, "M84\n");

    fclose(f);
#endif
  potrace_state_free(st);
  potrace_param_free(param);

  return 0;
}

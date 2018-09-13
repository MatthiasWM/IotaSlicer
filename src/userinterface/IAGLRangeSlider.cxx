//
// IAGLRangeSlider.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGLRangeSlider.h"

#include "IAGLButton.h"

#include <FL/gl.h>
#include <math.h>


/**
 Creates a new Fl_Slider widget using the given position,
 size, and label string. The default boxtype is FL_DOWN_BOX.
 */
IAGLRangeSlider::IAGLRangeSlider(int X, int Y, int W, int H, const char* L)
: Fl_Slider(X, Y, W, H, L) {
    type(FL_VERT_NICE_SLIDER);
}


void IAGLRangeSlider::draw()
{
    int xx = x()+w()/2, ww = 8;
    gl_draw_box(this, FL_DOWN_BOX, xx, y(), ww, h(), color());

    int X = x()+Fl::box_dx(box()), Y = y()+Fl::box_dy(box()),
    W = w()-Fl::box_dw(box()), H = h()-Fl::box_dh(box())-3*baseHeight;

    double rng = maximum() - minimum();
    int sx = X, sy = Y + (pLowValue-minimum())*(H/rng), sw = W, sh = baseHeight;
    int tx = X, ty = Y + (pHighValue-minimum())*(H/rng)+baseHeight*2, tw = W, th = baseHeight;
    gl_draw_box(this, btn==1 ? FL_DOWN_BOX : FL_UP_BOX, sx, sy, sw, sh, color());
    gl_draw_box(this, btn==2 ? FL_DOWN_BOX : FL_UP_BOX, sx+5, sy+baseHeight, sw-10, ty-sy-baseHeight, color());
    gl_draw_box(this, btn==3 ? FL_DOWN_BOX : FL_UP_BOX, tx, ty, tw, th, color());
}


int IAGLRangeSlider::handle(int event, int X, int Y, int W, int H)
{
    static int dy = 0;
    H -= 3*baseHeight;

    double rng = maximum() - minimum();
    int sy = Y + (pLowValue-minimum())*(H/rng), sh = baseHeight;
    int ty = Y + (pHighValue-minimum())*(H/rng)+baseHeight*2, th = baseHeight;

    int my, pl, ph;

    switch (event) {
        case FL_PUSH:
            btn = 0;
            if (Fl::event_inside(X, Y, W, H+3*baseHeight) && Fl::event_y()>=sy && Fl::event_y()<=ty+th) {
                if (Fl::event_y()>=ty) { btn = 3; dy = Fl::event_y() - ty; }
                else if (Fl::event_y()>=sy+sh) { btn = 2; dy = Fl::event_y() - sy; }
                else { btn = 1; dy = Fl::event_y() - sy; }
            } else {
                return 0;
            }
            // fall through ...
        case FL_DRAG:
            my = Fl::event_y() - dy;
            pl = pLowValue; ph = pHighValue;
            if (btn==1) {
                pLowValue = (my-Y)/(H/rng)+minimum();
                pLowValue = ::round(pLowValue);
                if (pLowValue<minimum()) pLowValue = minimum();
                if (pLowValue>maximum()) pLowValue = maximum();
                if (pLowValue>pHighValue) pHighValue = pLowValue;
            } else if (btn==2) {
                double d = pHighValue - pLowValue;
                pLowValue = (my-Y)/(H/rng)+minimum();
                pLowValue = ::round(pLowValue);
                if (pLowValue<minimum()) pLowValue = minimum();
                pHighValue = pLowValue + d;
                if (pHighValue>maximum()) { pHighValue = maximum(); pLowValue = pHighValue - d; }
            } else if (btn==3) {
                pHighValue = (my-2*baseHeight-Y)/(H/rng)+minimum();
                pHighValue = ::round(pHighValue);
                if (pHighValue<minimum()) pHighValue = minimum();
                if (pHighValue>maximum()) pHighValue = maximum();
                if (pLowValue>pHighValue) pLowValue = pHighValue;
            }
            if (pl!=pLowValue && ph!=pHighValue) {
                do_callback();
            }
            redraw();
            return 1;
        case FL_RELEASE:
            btn = 0;
            redraw();
            return 1;
        case FL_FOCUS :
        case FL_UNFOCUS :
            if (Fl::visible_focus()) {
                redraw();
                return 1;
            } else return 0;
        case FL_ENTER :
        case FL_LEAVE :
            return 1;
        default:
            return 0;
    }
    return 0;
}


int IAGLRangeSlider::handle(int event)
{
    if (event == FL_PUSH && Fl::visible_focus()) {
        Fl::focus(this);
        redraw();
    }

    return handle(event,
                  x()+Fl::box_dx(box()),
                  y()+Fl::box_dy(box()),
                  w()-Fl::box_dw(box()),
                  h()-Fl::box_dh(box()));
}


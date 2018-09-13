//
//  IAGLButton.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_GL_BUTTON_H
#define IA_GL_BUTTON_H

#include <FL/Fl.H>
#include <FL/Fl_Button.H>


/**
 * A button that can be used inside an OpenGL window.
 *
 * This is only a test for the upcomint Flgl library. Please do not use.
 *
 * \author Matthias Melcher
 */
class IAGLButton : public Fl_Button
{

public:

    IAGLButton(int x, int y, int w, int h, const char *label = 0);
    ~IAGLButton();
    void draw();

protected:
    void draw_box() const;
    void draw_box(Fl_Boxtype b, Fl_Color c) const;
    void draw_box(Fl_Boxtype b, int X, int Y, int W, int H, Fl_Color c) const;
    void draw_focus() const;
    void draw_focus(Fl_Boxtype B, int X, int Y, int W, int H) const;
};


void gl_draw_box(Fl_Widget const*, Fl_Boxtype b, int X, int Y, int W, int H, Fl_Color c);
void gl_draw_box(Fl_Widget const*);


#endif


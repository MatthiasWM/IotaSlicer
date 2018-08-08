//
// "$Id: hello.cxx 11782 2016-06-14 11:15:22Z AlbrechtS $"
//
// Hello, World! program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include "UserInterface.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

int main(int argc, char **argv) {
    Fl_Window *window = nullptr;
    window = make_window();
    window->show(argc, argv);
    return Fl::run();
}

//
// End of "$Id: hello.cxx 11782 2016-06-14 11:15:22Z AlbrechtS $".
//


//
//  IAModelView.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_MODEL_VIEW_H
#define IA_MODEL_VIEW_H

#include <FL/Fl_Gl_Window.H>

class IAMeshList;
class IASlice;

class IAModelView : public Fl_Gl_Window
{
    double dx, dy;
public:
    IAModelView(int x, int y, int w, int h, const char *l=0);
    int handle(int event);
    void draw(IAMeshList *meshList, IASlice *meshSlice);
    void draw();
    void writeSlice(int nDrops = 10, int interleave=4);
    void writePrnSlice();
    void clipToSlice(double z1, double z2);
    void dontClipToSlice();
};

#endif /* IA_MODEL_VIEW_H */



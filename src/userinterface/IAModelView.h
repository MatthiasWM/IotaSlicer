//
//  IAModelView.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_MODEL_VIEW_H
#define IA_MODEL_VIEW_H


#include "IACamera.h"
#include <FL/Fl_Gl_Window.H>


class IAMeshList;
class IASlice;
class IACamera;


/**
 * Class derived form FLTK to draw on screen via OpenGL.
 */
class IAModelView : public Fl_Gl_Window
{
public:
    IAModelView(int x, int y, int w, int h, const char *l=0);
    ~IAModelView() override;

    void setTopView();
    void setPerspectiveView();

protected:
    int handle(int event) override;
    void draw() override;
    void clipToSlice(double z1, double z2);
    void dontClipToSlice();
    void draw_children();
    void updateSlice();
    void initializeView();
    void initializeShaders();
    void beginTextures();
    void beginModels();
    void endModels();

private:
    IACamera *pPerspectiveCamera = nullptr;
    IACamera *pTopCamera = nullptr;
    IACamera *pCurrentCamera = nullptr;

    bool pShadersValid = false;
};


#endif /* IA_MODEL_VIEW_H */



//
//  IAModelView.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAModelView.h"

#include "../main.h"
#include "IACamera.h"
#include "../geometry/IAMesh.h"
#include "../geometry/IASlice.h"
#include "../printer/IAPrinter.h"

#include <math.h>

#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl_Slider.H>


/**
 * Create a new OpenGL scene view.
 *
 * When constructed, the widget will connect itself with the current Fl_Group
 * in FLTK.
 */
IAModelView::IAModelView(int x, int y, int w, int h, const char *l)
:   Fl_Gl_Window(x, y, w, h, l),
    pCamera( new IACamera(this) )
{
}


/**
 * Release all allocated resources.
 */
IAModelView::~IAModelView()
{
    delete pCamera;
}


/**
 * Handle all events that FLTK sends here.
 *
 * \todo FIXME: Handle all mouse input for manipulating the scene or the camera.
 * \todo Handle drag'n'drop events to add new models or textures.
 * \todo Handle copy and paste events.
 * \todo Handle context menus.
 */
int IAModelView::handle(int event) {
    // click to select
    // shift to drag
    // ctrl to rotate
    // scroll to dolly fraction of distance
    static int px = 0, py = 0;
    double dx, dy;
    switch (event) {
        case FL_MOUSEWHEEL:
            pCamera->dolly(Fl::event_dx()*1.5, Fl::event_dy()*1.5);
            redraw();
            return 1;
        case FL_PUSH:
            px = Fl::event_x();
            py = Fl::event_y();
            return 1;
        case FL_DRAG:
        case FL_RELEASE:
            dx = px - Fl::event_x();
            dy = py - Fl::event_y();
            px = Fl::event_x();
            py = Fl::event_y();
            if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == FL_SHIFT) {
                pCamera->rotate(dx, dy);
            } else if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == FL_CTRL) {
                pCamera->drag(dx, dy);
            } else if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == (FL_CTRL|FL_SHIFT)) {
                pCamera->dolly(dx, dy);
            }
            redraw();
            return 1;
    }
    return Fl_Gl_Window::handle(event);
}


/**
 * Draw the current mesh list and the current slice.
 *
 * \todo This function needs work.
 */
void IAModelView::draw(IAMeshList *meshList, IASlice *meshSlice)
{
    double z1 = zSlider1->value();
    double z2 = zSlider2->value();
    //---- draw the model using the near and far plane for clipping
    if (meshList) {
        clipToSlice(z1, z2);
#if 0
        meshList->drawFlat(false);
#else
        meshList->drawFlat(true);
#endif
    }

    //---- draw the lid outline
    if (meshSlice) {
        dontClipToSlice();
#if 0
        glDisable(GL_TEXTURE_2D);
        glColor3ub(128, 255, 255);
        meshSlice->drawLidEdge();
        glDisable(GL_TEXTURE_2D);
#else
        glEnable(GL_TEXTURE_2D);
        glColor3ub(255, 255, 255);
        meshSlice->drawLidEdge();
        glDisable(GL_TEXTURE_2D);
#endif
    }

    dontClipToSlice();
}


/**
 * Draw the entire scene.
 */
void IAModelView::draw()
{
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        //      setShaders();
    }

    if (!valid()) {
        static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        static GLfloat mat_shininess[] = { 50.0 };
        //static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
        static GLfloat light_position[] = { 1.0, -1.0, 1.0, 0.0 };
        static GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0};

        gl_font(FL_HELVETICA, 16 );

        glClearColor (0.9, 0.9, 0.9, 0.0);
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);

        glEnable(GL_BLEND);
        //      glBlendFunc(GL_ONE, GL_ZERO);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0,0,pixel_w(),pixel_h());

        if (texture) {
            static GLuint tex = 0;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->w(), texture->h(),
                         0, GL_RGB, GL_UNSIGNED_BYTE, *texture->data());
            glEnable(GL_TEXTURE_2D);
        }
        valid(1);
    }

    double z1 = zSlider1->value();
    double z2 = zSlider2->value();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pCamera->draw();
    glPushMatrix();

#if 0
    if (gShowSlice) {
        // show just the slice
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        //---- draw the untextured lid surface
        // change the z range to disable clipping
        dontClipToSlice();
#if 0
        gMeshSlice.drawFlat(false, 0.0, 1.0, 0.0);
#else
        gMeshSlice.drawFlat(false, 0.4, 0.4, 0.4);
#endif

        for (int n = 10; n>0; --n) {
            gMeshList.shrinkTo(0.2*n);
            IASlice meshSlice;
            meshSlice.generateFrom(gMeshList, zSlider1->value());
            draw(&gMeshList, &meshSlice);
        }
        gMeshList.shrinkTo(0.0);

        draw(&gMeshList, &gMeshSlice);

#if 0
        // set the z range again to enable drawing the shell
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
        glMatrixMode(GL_MODELVIEW);
        // the following code guarantees a hull of at least 1mm width
        //      double sd;
        //      glHint(GL_POLYGON_SMOOTH_HINT, );
        //      glDisable (GL_POLYGON_SMOOTH);
        //      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //      for (sd = 0.2; sd<gMinimumShell; sd+=0.2) {
        //        drawModelShrunk(Fl_WHITE, sd);
        //      }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
    } else {
        // show the 3d model
        gPrinter.draw();
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        gMeshList.drawFlat(gShowTexture);
    }
#endif

    gPrinter.draw();
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    if (gShowSlice) {
        double zPlane = zSlider1->value();
        // draw the opaque lower half of the model
        GLdouble equationLowerHalf[4] = { 0.0, 0.0, -1.0, zPlane };
        GLdouble equationUpperHalf[4] = { 0.0, 0.0, 1.0, -zPlane };
        glClipPlane(GL_CLIP_PLANE0, equationLowerHalf);
        glEnable(GL_CLIP_PLANE0);
        gMeshList.drawFlat(gShowTexture);
//        glEnable(GL_TEXTURE_2D);
//        gMeshList[0]->drawShrunk(FL_WHITE, -2.0);


        // draw the lid
        glDisable(GL_CLIP_PLANE0);
        gMeshSlice.drawFlat(1.0, 0.0, 0.0);
        glDisable(GL_DEPTH_TEST);
        gMeshSlice.drawLidEdge();
        glEnable(GL_DEPTH_TEST);

        // draw a texture map on the lid
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0, 1.0, 0.0, 0.1);
        glPushMatrix();
        glTranslated(gPrinter.pBuildVolumeOffset.x(), gPrinter.pBuildVolumeOffset.x(), 0.01);
        glBegin(GL_POLYGON);
        glVertex3d(gPrinter.pBuildVolumeMin.x(),
                   gPrinter.pBuildVolumeMin.y(),
                   zPlane);
        glVertex3d(gPrinter.pBuildVolumeMax.x(),
                   gPrinter.pBuildVolumeMin.y(),
                   zPlane);
        glVertex3d(gPrinter.pBuildVolumeMax.x(),
                   gPrinter.pBuildVolumeMax.y(),
                   zPlane);
        glVertex3d(gPrinter.pBuildVolumeMin.x(),
                   gPrinter.pBuildVolumeMax.y(),
                   zPlane);
        glEnd();
        glPopMatrix();

        // draw a ghoste upper half of the mode
        glClipPlane(GL_CLIP_PLANE0, equationUpperHalf);
        glEnable(GL_CLIP_PLANE0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        gMeshList.drawFlat(false, 0.6, 0.6, 0.6, 0.1);

        glDisable(GL_CULL_FACE);
        glDisable(GL_CLIP_PLANE0);
    } else {
        gMeshList.drawFlat(gShowTexture);
    }
    glPopMatrix();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, pixel_w(), 0, pixel_h(), -10, 10); // mm
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gl_color(FL_WHITE);
    char buf[1024];
    sprintf(buf, "Slice at %.4gmm", z1); gl_draw(buf, 10, 50);
    sprintf(buf, "%.4gmm thick", z2); gl_draw(buf, 10, 20);
}


/**
 * Enable clipping for the current slice (deprecated)
 */
void IAModelView::clipToSlice(double z1, double z2)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
    glMatrixMode(GL_MODELVIEW);
}


/**
 * Disable clipping for the current slice (deprecated)
 */
void IAModelView::dontClipToSlice()
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
    glMatrixMode(GL_MODELVIEW);
}


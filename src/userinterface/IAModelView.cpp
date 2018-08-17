//
//  IAModelView.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAModelView.h"

#include "../Iota.h"
#include "IACamera.h"
#include "../geometry/IAMesh.h"
#include "../geometry/IASlice.h"
#include "../printer/IAPrinter.h"
#include "../userinterface/IAGUIMain.h"

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
    pPerspectiveCamera( new IAPerspectiveCamera(this) ),
    pTopCamera( new IAOrthoCamera(this, 0) ),
    pCurrentCamera( pPerspectiveCamera )
{
}


/**
 * Release all allocated resources.
 */
IAModelView::~IAModelView()
{
    delete pCurrentCamera;
}


/**
 * Handle all events that FLTK sends here.
 *
 * \todo FIXME: Handle all mouse input for manipulating the scene or the camera.
 * \todo Handle drag'n'drop events to add new models or textures.
 * \todo Handle copy and paste events.
 * \todo Handle context menus.
 */
int IAModelView::handle(int event)
{
    if (Fl_Window::handle(event))
        return 1;

    // click to select
    // shift to drag
    // ctrl to rotate
    // scroll to dolly fraction of distance
    static int px = 0, py = 0;
    double dx, dy;
    switch (event) {
        case FL_MOUSEWHEEL:
            pCurrentCamera->dolly(Fl::event_dx()*1.5, Fl::event_dy()*1.5);
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
                pCurrentCamera->rotate(dx, dy);
            } else if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == FL_CTRL) {
                pCurrentCamera->drag(dx, dy);
            } else if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == (FL_CTRL|FL_SHIFT)) {
                pCurrentCamera->dolly(dx, dy);
            }
            redraw();
            return 1;
        case FL_DND_ENTER: return 1;
        case FL_DND_DRAG: return 1;
        case FL_DND_RELEASE: return 1;
        case FL_PASTE:
            Iota.loadAnyFileList(Fl::event_text());
            break;
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
//    double z1 = zSlider1->value();
//    double z2 = zSlider2->value();
    //---- draw the model using the near and far plane for clipping
    if (meshList) {
//        clipToSlice(z1, z2);
#if 0
        meshList->drawFlat(false);
#else
        meshList->drawFlat(true);
#endif
    }

    //---- draw the lid outline
    if (meshSlice) {
//        dontClipToSlice();
#if 0
        glDisable(GL_TEXTURE_2D);
        glColor3ub(128, 255, 255);
        meshSlice->drawLidEdge();
        glDisable(GL_TEXTURE_2D);
#else
        glEnable(GL_TEXTURE_2D);
        glColor3ub(128, 128, 128);
        meshSlice->drawLidEdge();
        glDisable(GL_TEXTURE_2D);
#endif
    }

//    dontClipToSlice();
}


/**
 * Draw the entire scene.
 */
void IAModelView::draw()
{
    if (Iota.gShowSlice && Iota.gMeshSlice.pCurrentZ!=zSlider1->value()) {
        Iota.gMeshSlice.generateLidFrom(*Iota.gMeshList, zSlider1->value());
    }
    static Fl_RGB_Image *lTexture = nullptr;
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        //      setShaders();
    }

    if (!valid()) {
        gl_font(FL_HELVETICA, 16 );
        static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        static GLfloat mat_shininess[] = { 50.0 };
        static GLfloat light_position[] = { 1.0, -1.0, 1.0, 0.0 };
        static GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0};

        glClearColor (0.9, 0.9, 0.9, 0.0);
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);

        glEnable(GL_BLEND);
        // glBlendFunc(GL_ONE, GL_ZERO);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0,0,pixel_w(),pixel_h());

        valid(1);
    }
    
    static GLuint tex = 0;
    if (lTexture != Iota.texture) {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Iota.texture->w(), Iota.texture->h(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, *Iota.texture->data());
        glEnable(GL_TEXTURE_2D);
        lTexture = Iota.texture;
    }
    if (Iota.texture) {
        glBindTexture(GL_TEXTURE_2D, tex);
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    double z1 = zSlider1->value();
    double z2 = zSlider2->value();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pCurrentCamera->draw();
    glPushMatrix();


    Iota.gPrinter.draw();
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    if (Iota.gShowSlice) {
        double zPlane = zSlider1->value();
        // draw the opaque lower half of the model
        GLdouble equationLowerHalf[4] = { 0.0, 0.0, -1.0, zPlane-0.05 };
        GLdouble equationUpperHalf[4] = { 0.0, 0.0, 1.0, -zPlane+0.05 };
        glClipPlane(GL_CLIP_PLANE0, equationLowerHalf);
        glEnable(GL_CLIP_PLANE0);
        Iota.gMeshList->drawFlat(Iota.gShowTexture);
//        glEnable(GL_TEXTURE_2D);
//        gMeshList[0]->drawShrunk(FL_WHITE, -2.0);


#if 1   // draw the shell
        // FIXME: this messes up tesselation for the lid!
        // FIXME: we do not need to tesselate at all!
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glLineWidth(8.0);
        for (int n = 20; n>0; --n) {
            Iota.gMeshList->shrinkBy(0.1*n);
            IASlice meshSlice;
            meshSlice.generateOutlineFrom(*Iota.gMeshList, zSlider1->value());
            draw(Iota.gMeshList, &meshSlice);
        }
        Iota.gMeshList->shrinkBy(0.0);
        glLineWidth(1.0);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
#endif

#if 0   // draw the lid
        glDisable(GL_CLIP_PLANE0);
        gMeshSlice.drawFlat(1.0, 0.9, 0.9);
#endif

#if 0
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
#endif

        // draw a ghoste upper half of the mode
        glClipPlane(GL_CLIP_PLANE0, equationUpperHalf);
        glEnable(GL_CLIP_PLANE0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        Iota.gMeshList->drawFlat(false, 0.6, 0.6, 0.6, 0.1);

        glDisable(GL_CULL_FACE);
        glDisable(GL_CLIP_PLANE0);
    } else {
        Iota.gMeshList->drawFlat(Iota.gShowTexture);
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

    draw_children();
}

/**
 * Draw FLTK child widgets
 */
void IAModelView::draw_children()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, w()-0.5, h()-0.5, -0.5, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    Fl_Widget*const* a = array();
    for (int i=children(); i--;) {
        Fl_Widget& o = **a++;
        draw_child(o);
        draw_outside_label(o);
    }
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


void IAModelView::setTopView()
{
    pCurrentCamera = pTopCamera;
    redraw();
}


void IAModelView::setPerspectiveView()
{
    pCurrentCamera = pPerspectiveCamera;
    redraw();
}



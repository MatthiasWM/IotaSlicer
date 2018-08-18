//
//  IASceneView.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASceneView.h"

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
IASceneView::IASceneView(int x, int y, int w, int h, const char *l)
:   Fl_Gl_Window(x, y, w, h, l),
    pPerspectiveCamera( new IAPerspectiveCamera(this) ),
    pTopCamera( new IAOrthoCamera(this, 0) ),
    pCurrentCamera( pPerspectiveCamera )
{
    IAVector3d interest = {
        Iota.gPrinter.pBuildVolume.x() * 0.5,
        Iota.gPrinter.pBuildVolume.y() * 0.5,
        Iota.gPrinter.pBuildVolume.z() * 0.333
    };
    pPerspectiveCamera->setInterest(interest);
    pTopCamera->setInterest(interest);
}


/**
 * Release all allocated resources.
 */
IASceneView::~IASceneView()
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
int IASceneView::handle(int event)
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
 * Update the preview slice if the Z slice level changed and it needs to be rendered.
 * \todo This should be a function of IASlice.
 */
void IASceneView::updateSlice()
{
    // TODO: refactor into slice class
    if ( Iota.gShowSlice && Iota.gMeshSlice.pCurrentZ != zSlider1->value() ) {
        Iota.gMeshSlice.pCurrentZ = zSlider1->value();
        Iota.gMeshSlice.clear();
        if (Iota.pMesh) {
            Iota.gMeshSlice.generateFlange( Iota.pMesh );
            Iota.gMeshSlice.tesselateLidFromFlange();
//          Iota.gMeshSlice.generateLid(Iota.pMesh, zSlider1->value());
        }
    }
}


/**
 * Initialize all shaders that we might want to use.
 */
void IASceneView::initializeShaders()
{
    if (!pShadersValid) {
//      setShaders();
        pShadersValid = true;
    }
}


/**
 * Initialize all standard OpenGL settings of the current view.
 */
void IASceneView::initializeView()
{
    if (!valid()) {
        gl_font(FL_HELVETICA, 16 );
        static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        static GLfloat mat_shininess[] = { 50.0 };
        static GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0};
        static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0};

        glClearColor (0.9, 0.9, 0.9, 0.0);
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
//        glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);

        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_NORMALIZE);

        glEnable(GL_BLEND);
        // glBlendFunc(GL_ONE, GL_ZERO);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0,0,pixel_w(),pixel_h());

        if (!pShadersValid) initializeShaders();

        valid(1);
    }
}


/**
 * Initialize and activate textures for rendering.
 * \todo must move into a texture class
 */
void IASceneView::beginTextures()
{
    static Fl_RGB_Image *lTexture = nullptr;
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
}


/**
 * Start drawing models.
 */
void IASceneView::beginModels()
{
    // initialize model drawing
    glPushMatrix();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}


/**
 * End drawing models.
 */
void IASceneView::endModels()
{
    glPopMatrix();
}


/**
 * Draw the entire scene.
 */
void IASceneView::draw()
{
    if (!valid()) initializeView();

    // initialize the frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateSlice();
    beginTextures();

    pCurrentCamera->draw();

    static GLfloat light_position0[] = { 4.0, -1.0, 2.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    static GLfloat light_position1[] = { -3.0, -1.0, -2.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

    // Draw the OpenGL origin.
    glDisable(GL_LIGHTING);
    const float sze = 5.0f;
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex3f( sze,  0.0, 0.0);
    glVertex3f( 0.0,  sze, 0.0);
    glVertex3f(-sze,  0.0, 0.0);
    glVertex3f( 0.0, -sze, 0.0);
    glLineWidth(1.0);
    glEnd();
    glEnable(GL_LIGHTING);

    glPushMatrix();
    Iota.gPrinter.draw();
    glPopMatrix();

    beginModels();
    if (Iota.pMesh) {
        glPushMatrix();
        glTranslated(Iota.pMesh->position().x(), Iota.pMesh->position().y(), Iota.pMesh->position().z());
        if (Iota.gShowSlice) {
            Iota.pMesh->drawSliced(zSlider1->value());
        } else {
            Iota.pMesh->drawFlat(Iota.gShowTexture);
        }
        glPopMatrix();
    }
    endModels();

    Iota.gMeshSlice.drawFlat(false, 1.0, 0.0, 0.0);
    Iota.gMeshSlice.drawFlange();

    draw_children(); // draw FLTK user interface
}


/**
 * Draw FLTK child widgets
 */
void IASceneView::draw_children()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, w()-0.5, h()-0.5, -0.5, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // FIXME: buttons disappear when we start to slice until we resize the window
    // so, generating a slice or loading the texture for the first time seems to cause this!
    Fl_Widget*const* a = array();
    for (int i=children(); i--;) {
        Fl_Widget& o = **a++;
        draw_child(o);
        draw_outside_label(o);
    }
}


void IASceneView::setTopView()
{
    pCurrentCamera = pTopCamera;
    redraw();
}


void IASceneView::setPerspectiveView()
{
    pCurrentCamera = pPerspectiveCamera;
    redraw();
}



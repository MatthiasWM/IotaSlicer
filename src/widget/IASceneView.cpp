//
//  IASceneView.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IASceneView.h"

#include "Iota.h"
#include "view/IAGUIMain.h"
#include "widget/IACamera.h"
#include "widget/IAGLRangeSlider.h"
#include "geometry/IAMesh.h"
#include "geometry/IAMeshSlice.h"
#include "printer/IAPrinter.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"

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
        Iota.pCurrentPrinter->pPrintVolume.x() * 0.5,
        Iota.pCurrentPrinter->pPrintVolume.y() * 0.5,
        Iota.pCurrentPrinter->pPrintVolume.z() * 0.2
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
 * \param event the FLTK event that we need to handle
 *
 * \return 1 if we handled the event, 0 if we did not know what to do with it.
 *
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
            if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == 0) {
                pCurrentCamera->rotate(dx, dy);
            } else if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == FL_CTRL) {
                pCurrentCamera->rotate(dx, dy);
            } else if ( (Fl::event_state()&(FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == FL_SHIFT) {
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
            Iota.loadAnyFile(Fl::event_text());
            return 1;
    }
    return Fl_Gl_Window::handle(event);
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
	Iota.Error.clear();
    /** \todo The code below should really only be called once in the entire runtime of the app. */
	if (initializeOpenGL() == false) {
		Iota.Error.showDialog();
		return;
	}


    if (!valid()) {
        gl_font(FL_HELVETICA, 16 );
        IA_HANDLE_GL_ERRORS();
        static GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        static GLfloat mat_shininess[] = { 50.0f };
        static GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f};
        static GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f};

        glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
        glShadeModel(GL_SMOOTH);

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
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        IA_HANDLE_GL_ERRORS();

        glViewport(0,0,pixel_w(),pixel_h());
        IA_HANDLE_GL_ERRORS();

        if (!pShadersValid) initializeShaders();

        valid(1);
    }
}


/**
 * Initialize and activate textures for rendering.
 *
 * \todo must move into a texture class
 */
void IASceneView::beginTextures()
{
    static Fl_RGB_Image *lTexture = nullptr;
//    static GLUInt tex = 0;
    if (lTexture != Iota.texture) {
        IA_HANDLE_GL_ERRORS();
        glGenTextures(1, &tex);
        IA_HANDLE_GL_ERRORS();
        glBindTexture(GL_TEXTURE_2D, tex);
        IA_HANDLE_GL_ERRORS();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        IA_HANDLE_GL_ERRORS();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        IA_HANDLE_GL_ERRORS();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Iota.texture->w(), Iota.texture->h(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, *Iota.texture->data());
        IA_HANDLE_GL_ERRORS();
        glEnable(GL_TEXTURE_2D);
        IA_HANDLE_GL_ERRORS();
        lTexture = Iota.texture;
    }
    if (Iota.texture) {
        glBindTexture(GL_TEXTURE_2D, tex);
        IA_HANDLE_GL_ERRORS();
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
        IA_HANDLE_GL_ERRORS();
    }
}


/**
 * Start drawing models.
 */
void IASceneView::beginModels()
{
    // initialize model drawing
    IA_HANDLE_GL_ERRORS();
    glPushMatrix();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    IA_HANDLE_GL_ERRORS();
}


/**
 * End drawing models.
 */
void IASceneView::endModels()
{
    IA_HANDLE_GL_ERRORS();
    glPopMatrix();
    IA_HANDLE_GL_ERRORS();
}


/**
 * Draw the entire scene.
 *
 * This is called whenever FLTK thinks that it needs to redisplay this widget.
 */
void IASceneView::draw()
{
    if (!valid()) initializeView();

    IA_HANDLE_GL_ERRORS();
    // initialize the frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    IA_HANDLE_GL_ERRORS();
    glEnable(GL_BLEND);
    IA_HANDLE_GL_ERRORS();
    beginTextures();

    pCurrentCamera->draw();
    IA_HANDLE_GL_ERRORS();

    static GLfloat light_position0[] = { 4.0, -1.0, 2.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    static GLfloat light_position1[] = { -3.0, -1.0, -2.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

    // Draw the OpenGL origin.
    IA_HANDLE_GL_ERRORS();
    glDisable(GL_LIGHTING);
    IA_HANDLE_GL_ERRORS();
    const float sze = 5.0f;
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex3f( sze,  0.0, 0.0);
    glVertex3f( 0.0,  sze, 0.0);
    glVertex3f(-sze,  0.0, 0.0);
    glVertex3f( 0.0, -sze, 0.0);
    glEnd();
    glLineWidth(1.0);
    glEnable(GL_LIGHTING);
    IA_HANDLE_GL_ERRORS();

    glPushMatrix();
    Iota.pCurrentPrinter->draw();
    glPopMatrix();

    beginModels();
    if (Iota.pMesh) {
#if 0
        glPushMatrix();
        glTranslated(Iota.pMesh->position().x(), Iota.pMesh->position().y(), Iota.pMesh->position().z());
        if (Iota.gShowSlice) {
            Iota.pMesh->drawSliced(zRangeSlider->lowValue() * Iota.pCurrentPrinter->pLayerHeight);
        } else {
//            Iota.pMesh->drawEdges();
            Iota.pMesh->drawFlat(Iota.gShowTexture);
//            Iota.pMesh->drawEdges();
        }
        glPopMatrix();
#else
        IA_HANDLE_GL_ERRORS();
        glPushMatrix();
        glTranslated(Iota.pMesh->position().x(), Iota.pMesh->position().y(), Iota.pMesh->position().z());
        Iota.pMesh->drawSliced(zRangeSlider->lowValue() * Iota.pCurrentPrinter->layerHeight());
        glPopMatrix();
        IA_HANDLE_GL_ERRORS();

        if (Iota.pCurrentPrinter)
            Iota.pCurrentPrinter->drawPreview(zRangeSlider->lowValue(),
                                              zRangeSlider->highValue());
        IA_HANDLE_GL_ERRORS();
        glPushMatrix();
        glTranslated(Iota.pMesh->position().x(), Iota.pMesh->position().y(), Iota.pMesh->position().z());
        Iota.pMesh->drawSlicedGhost(zRangeSlider->lowValue() * Iota.pCurrentPrinter->layerHeight());
        glPopMatrix();
        IA_HANDLE_GL_ERRORS();
#endif

    }
    endModels();

//    Iota.pCurrentPrinter->gSlice.drawFlat(false, 1.0, 0.0, 0.0);
//    Iota.pCurrentPrinter->gSlice.drawRim();

    // draw the texture generated by tesselating the slice
    /** \todo this is not working as expected! */
    // Iota.pCurrentPrinter->gSlice.drawFramebuffer();

    // draw the machine toolpath (or whatever kind of preview the printer has)
//    if (Iota.pCurrentPrinter)
//        Iota.pCurrentPrinter->drawPreview(zRangeSlider->lowValue(),
//                                          zRangeSlider->highValue());

    // draw FLTK user interface
    draw_children();
}


/**
 * A now obsolte attempt to make sure that all FLTK children are
 * drawn correctly.
 */
void IASceneView::redraw()
{
    super::redraw();
}


/**
 * Draw FLTK child widgets
 */
void IASceneView::draw_children()
{
    if (!child(0)) return;
    
    IA_HANDLE_GL_ERRORS();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, w()-0.5, h()-0.5, -0.5, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    Fl_Window::make_current();
//    fl_push_no_clip();
//    make_current();
    Fl_Widget*const* a = array();
    for (int i=children(); i--;) {
        Fl_Widget& o = **a++;
        draw_child(o);
        draw_outside_label(o);
    }
    if (!Iota.pMesh) {
        gl_color(FL_BLACK);
        gl_font(FL_HELVETICA, 18);
        /** \todo Fix in FLTK: Multiline output will write bottom to top instead of top to bottom!
         *        This assumes that we use top/left origin like FLTK does.
         */
        gl_draw("Drag and drop STL files here", 0, 0, w(), h(), FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    }
//    fl_pop_clip();
    IA_HANDLE_GL_ERRORS();
}


/**
 * Change camera to ortho top view.
 */
void IASceneView::setTopView()
{
    pCurrentCamera = pTopCamera;
    redraw();
}


/**
 * Change camera to perspective view.
 */
void IASceneView::setPerspectiveView()
{
    pCurrentCamera = pPerspectiveCamera;
    redraw();
}



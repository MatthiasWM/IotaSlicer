//
//  IASceneView.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_MODEL_VIEW_H
#define IA_MODEL_VIEW_H


#include "IACamera.h"
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>


class IAMeshList;
class IASlice;
class IACamera;


/**
 * Class derived form FLTK to draw on screen via OpenGL.
 *
 * The scene view show all 3D objects in our 3D printing world. SceneView
 * manages a number of cameras, some hovering user interface elements, and
 * general world representation.
 *
 * A scene view is composed from:
 *  - an active camera (and multiple alternative cameras)
 *  - an active printer representation (and the option to choose other printers)
 *  - one or more models (triangle meshes) that will be printed
 *  - one current slicing plane (and possibly more cached planes)
 *  - one toolpath (possibly segmented into layers)
 *  - 3D manipulaters for arranging and otherwise manipulating models
 *
 * The origin of the 3D view is the global coordinate system as it is used in
 * CNC manufacturing and often copied in 3D printing. It is located on the build
 * platform in the bottom, left, front corner. X points right, Y points
 * backwards, Z points up, using the right-hand rule.
 */
class IASceneView : public Fl_Gl_Window
{
public:
    IASceneView(int x, int y, int w, int h, const char *l=0);
    ~IASceneView() override;

    void setTopView();
    void setPerspectiveView();

    GLuint tex = 0;

protected:
    int handle(int event) override;
    void draw() override;
    void draw_children();
    void updateSlice();
    void initializeView();
    void initializeShaders();
    void beginTextures();
    void beginModels();
    void endModels();

private:
    /// The prespective camera is one option for viewing the scene.
    IACamera *pPerspectiveCamera = nullptr;
    /// The top camera is orthogonal and should help inspect slices (not on belt machines!)
    IACamera *pTopCamera = nullptr;
    /// The current camera points to one of the cameras above.
    IACamera *pCurrentCamera = nullptr;
    // This is set when the class has initialized all OpenGL shaders.
    bool pShadersValid = false;
};


#endif /* IA_MODEL_VIEW_H */



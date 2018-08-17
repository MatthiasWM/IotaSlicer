//
//  IACamera.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IACamera.h"

#include "Iota.h"
#include "IASceneView.h"
#include "../geometry/IAMath.h"

#include <FL/gl.h>
#include <FL/glu.h>

#include <math.h>

// FIXME: where does MS define M_PI?
#ifndef M_PI
#define M_PI 3.141592654
#endif


/**
 * Create a camera superclass.
 */
IACamera::IACamera(IASceneView *view)
:   pView( view )
{
}


/**
 * Create a simple perspective camera.
 */
IAPerspectiveCamera::IAPerspectiveCamera(IASceneView *view)
:   super( view )
{
}


/**
 * User wants to rotate the camera.
 */
void IAPerspectiveCamera::rotate(double dx, double dy)
{
    pZRotation += dx/100.0;
    pXRotation += dy/100.0;

    while (pZRotation>=M_PI*2.0) pZRotation -= M_PI*2.0;
    while (pZRotation<0.0) pZRotation += M_PI*2.0;

    if (pXRotation>M_PI*0.48) pXRotation = M_PI*0.48;
    if (pXRotation<-M_PI*0.48) pXRotation = -M_PI*0.48;
}


/**
 * User wants to drag the camera around.
 */
void IAPerspectiveCamera::drag(double dx, double dy)
{
    IAVector3d offset(0.5*dx, -0.5*dy, 0);
    offset.xRotate(pXRotation);
    offset.yRotate(pZRotation);
    pInterest += offset;
}


/**
 * User wants to get closer to the point of interest.
 */
void IAPerspectiveCamera::dolly(double dx, double dy)
{
    pDistance = pDistance * (1.0+0.01*dy);
    if (pDistance<5.0) pDistance = 5.0;
}


/**
 * Emit OpenGL commands to load the viewing and model matrices.
 */
void IAPerspectiveCamera::draw()
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    IAVector3d position = IAVector3d(0.0, 0.0, pDistance);
    position.xRotate(pXRotation);
    position.yRotate(pZRotation);
    position += pInterest;

    double dist = position.length();
    double aspect = (double(pView->pixel_w()))/(double(pView->pixel_h()));
    double nearPlane = max(dist-Iota.gPrinter.pBuildVolumeRadius, 5.0);
    double farPlane = dist+Iota.gPrinter.pBuildVolumeRadius;
    gluPerspective(50.0, aspect, nearPlane, farPlane);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(position.x(), position.y(), position.z(),
              pInterest.x(), pInterest.y(), pInterest.z(),
              0.0, 1.0, 0.0);

    glRotated(-90, 1.0, 0.0, 0.0);
}



/**
 * Create a simple perspective camera.
 */
IAOrthoCamera::IAOrthoCamera(IASceneView *view, int direction)
:   super( view )
{
}


/**
 * User wants to rotate the camera.
 */
void IAOrthoCamera::rotate(double dx, double dy)
{
    // Do we want to allow rotation?
}


/**
 * User wants to drag the camera around.
 */
void IAOrthoCamera::drag(double dx, double dy)
{
    IAVector3d offset(-dx, dy, 0);
    offset *= 2.0*pZoom/pView->w();
    pInterest += offset;
}


/**
 * User wants to get closer to the point of interest.
 */
void IAOrthoCamera::dolly(double dx, double dy)
{
    pZoom = pZoom * (1.0+0.01*dy);
    if (pZoom<1.0) pZoom = 1.0;
    if (pZoom>2.0*Iota.gPrinter.pBuildVolumeRadius) pZoom = 2.0*Iota.gPrinter.pBuildVolumeRadius;
}


/**
 * Emit OpenGL commands to load the viewing and model matrices.
 */
void IAOrthoCamera::draw()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double aspect = (double(pView->pixel_w()))/(double(pView->pixel_h()));
    glOrtho(-pZoom*aspect, pZoom*aspect, -pZoom, pZoom, -Iota.gPrinter.pBuildVolumeRadius, Iota.gPrinter.pBuildVolumeRadius);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(pInterest.x(), pInterest.y(), pInterest.z());
}


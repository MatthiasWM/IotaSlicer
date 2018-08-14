//
//  IACamera.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IACamera.h"

#include "main.h"
#include "IAModelView.h"

#include <FL/gl.h>
#include <FL/glu.h>

#include <math.h>

// FIXME: where does MS define M_PI?
#ifndef M_PI
#define M_PI 3.141592654
#endif


IACamera::IACamera(IAModelView *view)
:   pView( view )
{
}


void IACamera::rotate(double dx, double dy)
{
    pZRotation += dx/100.0;
    pXRotation += dy/100.0;

    while (pZRotation>=M_PI*2.0) pZRotation -= M_PI*2.0;
    while (pZRotation<0.0) pZRotation += M_PI*2.0;

    if (pXRotation>M_PI*0.48) pXRotation = M_PI*0.48;
    if (pXRotation<-M_PI*0.48) pXRotation = -M_PI*0.48;
}


void IACamera::drag(double dx, double dy)
{
    IAVector3d offset(0.5*dx, -0.5*dy, 0);
    offset.xRotate(pXRotation);
    offset.yRotate(pZRotation);
    pInterest += offset;
}


void IACamera::dolly(double dx, double dy)
{
    pDistance = pDistance * (1.0+0.01*dy);
    if (pDistance<5.0) pDistance = 5.0;
}


void IACamera::draw()
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    IAVector3d position = IAVector3d(0.0, 0.0, pDistance);
    position.xRotate(pXRotation);
    position.yRotate(pZRotation);
    position += pInterest;

    double dist = position.length();
    double aspect = (double(pView->pixel_w()))/(double(pView->pixel_h()));
    double nearPlane = max(dist-gPrinter.pBuildVolumeRadius, 5.0);
    double farPlane = dist+gPrinter.pBuildVolumeRadius;
    gluPerspective(50.0, aspect, nearPlane, farPlane);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(position.x(), position.y(), position.z(),
              pInterest.x(), pInterest.y(), pInterest.z(),
              0.0, 1.0, 0.0);

    glRotated(-90, 1.0, 0.0, 0.0);
}


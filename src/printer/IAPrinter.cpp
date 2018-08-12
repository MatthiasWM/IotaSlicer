//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinter.h"

#include <math.h>

#include <FL/gl.h>
#include <FL/glu.h>


// M3D Crane
//   Build Volume: 214 X 214 X 230 mm

IAPrinter::IAPrinter()
{
}

void IAPrinter::draw()
{
    // draw printing volume using OpenGL
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glLoadIdentity();
    glTranslated(pBuildVolumeOffset.x(), pBuildVolumeOffset.y(), pBuildVolumeOffset.z());

    glColor3ub(230, 230, 230);

    // top
    glBegin(GL_LINE_LOOP);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glEnd();

    // corners
    glBegin(GL_LINES);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMax.z());
    glEnd();

    // bottom frame
    glBegin(GL_LINE_LOOP);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glEnd();

    // bottom plate
    glColor3ub(128, 128, 128);
    glBegin(GL_POLYGON);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glEnd();

    // bottom grid
    glColor3ub(200, 200, 200);
    int i;
    int xmin = (int)ceil(pBuildVolumeMin.x()/10.0)*10;
    int xmax = (int)floor(pBuildVolumeMax.x()/10.0)*10;
    glBegin(GL_LINES);
    for (i=xmin; i<=xmax; i+=10) {
        glVertex3d(i, pBuildVolumeMin.y(), pBuildVolumeMin.z());
        glVertex3d(i, pBuildVolumeMax.y(), pBuildVolumeMin.z());
    }
    glEnd();
    int ymin = (int)ceil(pBuildVolumeMin.y()/10.0)*10;
    int ymax = (int)floor(pBuildVolumeMax.y()/10.0)*10;
    glBegin(GL_LINES);
    for (i=ymin; i<=ymax; i+=10) {
        glVertex3d(pBuildVolumeMin.x(), i, pBuildVolumeMin.z());
        glVertex3d(pBuildVolumeMax.x(), i, pBuildVolumeMin.z());
    }
    glEnd();

    // origin and coordinate system
    glBegin(GL_LINES);
    glColor3ub(255, 0, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(10, 0, 0);
    glColor3ub(0, 255, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 10, 0);
    glColor3ub(0, 0, 255);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 0, 10);
    glEnd();
}


//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinter.h"

#include <math.h>

#include <FL/gl.h>
#include <FL/glu.h>


/**
 Create a default printer.

 For example, an M3D Crane:
    Build Volume: 214 X 214 X 230 mm, bottom front left is 0
    The Crane is designed to extrude at a rate of 5.7 mm^3/s
 Flow Rate (mm^3/s) = Feedrate (mm/s) (Filament Cross-section) (mm^2)*
 Filament Cross-section = pi ((Filament Diameter) / 2)^2 __Filament Cross-section for 1.75mm filament: 2.405 mm^2*

 For Example: You send the command G1 E100 F200, where you extrude 100mm of filament at 200 mm/min. You are using 1.75 mm filament.

 Flow Rate = 8 mm/s = pi (1.75 / 2)^2 (200 / 60) = 2.405 3.333*

 Remember that the G1 move command feedrate parameter, F, uses mm per minute. So divide the feedrate by 60 to obtain the feedrate in mm per second.

 Printing Flow Rate

 When printing, the flow rate depends on your layer height, nozzle diameter and print speed.

 Flow Rate (mm^3/s) = (Extrusion Width)(mm) (Layer Height)(mm) Print Speed (mm/s) Extrusion Width is ~120% of nozzle diameter

 For Example: You have a 0.5 mm nozzle mounted and you are printing at 0.25mm layer height at a print speed of 30 mm/s.

 Extrusion Width = 0.6 mm = 1.2 0.5 Flow Rate = 4.5 mm^3/s = 0.6 0.25 * 30


 // The tool 0 box is mounted in a very bad place and can just barely be reached.
 // We need to drive the head in a circle to actually wipe, and make a bee line
 // so that waste goes into the waste box. Phew. Also, when usug repetier FW, the
 // width needs to be corrected so that we reach the waste container at all.
 // Wipe tool 0
 // Box:     200, 0, 10
 // Wipe:    180, 0, 10

 // The tool 1 box is located a little bit better, but in order to reach it,
 // tool 0 must be active. I am not even sure if we can send a command to extrude
 // to head 1 without changing the position back onto the bed. In effect, we would
 // have change the minimum position for x to minus whatever the offest is between heads.
 // Maybe that's why in the original firmware, the *right* head is tool 0, and
 // the *left* head ist tool 1?
 // Wipe tool 1
 // Box:     T0! 0, 0, 10
 // Wipe:    T0! 0, 20, 10
 // but can we extrude T1 without changing the position? Maybe by using relative
 // positioning?

 */
IAPrinter::IAPrinter()
{
}


/**
 Draw a minimal shape representing the printer in the scene.
 */
void IAPrinter::draw()
{
    // draw printing volume using OpenGL
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // draw the printer so that the bottom left front corner of the build
    // platform is in the OpenGL origin
    glPushMatrix();
    glTranslated(-pBuildVolumeMin.x(), -pBuildVolumeMin.y(), -pBuildVolumeMin.z());

    // bottom plate
    glColor3ub(200, 200, 200);
    glBegin(GL_POLYGON);
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMin.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMax.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glVertex3d(pBuildVolumeMin.x(), pBuildVolumeMax.y(), pBuildVolumeMin.z());
    glEnd();

    // bottom grid
    glColor3ub(70, 70, 70);
    int i;
    int xmin = (int)ceil(pBuildVolumeMin.x()/10.0)*10;
    int xmax = (int)floor(pBuildVolumeMax.x()/10.0)*10;
    glBegin(GL_LINES);
    for (i=xmin; i<=xmax; i+=10) {
        if (i==0) { glEnd(); glLineWidth(2.5); glBegin(GL_LINES); }
        glVertex3d(i, pBuildVolumeMin.y(), pBuildVolumeMin.z());
        glVertex3d(i, pBuildVolumeMax.y(), pBuildVolumeMin.z());
        if (i==0) { glEnd(); glLineWidth(1.0); glBegin(GL_LINES); }
    }
    glEnd();
    int ymin = (int)ceil(pBuildVolumeMin.y()/10.0)*10;
    int ymax = (int)floor(pBuildVolumeMax.y()/10.0)*10;
    glBegin(GL_LINES);
    for (i=ymin; i<=ymax; i+=10) {
        if (i==0) { glEnd(); glLineWidth(2.5); glBegin(GL_LINES); }
        glVertex3d(pBuildVolumeMin.x(), i, pBuildVolumeMin.z());
        glVertex3d(pBuildVolumeMax.x(), i, pBuildVolumeMin.z());
        if (i==0) { glEnd(); glLineWidth(1.0); glBegin(GL_LINES); }
    }
    glEnd();

    glColor3ub(70, 70, 70);

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

    // origin and coordinate system
    glLineWidth(2.5);
    glBegin(GL_LINES);
    glColor3ub(255, 0, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(50, 0, 0);
    glColor3ub(0, 255, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 50, 0);
    glColor3ub(0, 0, 255);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 0, 50);
    glEnd();

    glLineWidth(1.0);
    glPopMatrix();
}


//
//  IAToolpath.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAToolpath.h"

#include "Iota.h"

#include <FL/gl.h>

static IAVector3d first, prev;
extern "C" {
    void startPath(double x, double y) { first.set(x, y, 100); prev = first;  }
    void continuePath(double x, double y)  {
        IAVector3d next(x, y, 100);
        Iota.pToolpath->pList.push_back(new IAToolpathMotion(prev, next));
        prev = next;
    }
    void closePath() {
        Iota.pToolpath->pList.push_back(new IAToolpathMotion(prev, first));
    }
}
    

IAMachineToolpath::IAMachineToolpath()
{
}


/**
 */
IAToolpath::IAToolpath()
{
}


IAToolpath::~IAToolpath()
{
    clear();
}


void IAToolpath::clear()
{
    for (auto e: pList) {
        delete e;
    }
    pList.clear();
}


void IAToolpath::draw()
{
    glLineWidth(5.0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
//    {
//        const float sze = 50.0f;
//        glColor3f(0.0, 0.0, 0.0);
//        glLineWidth(3.0);
//        glBegin(GL_LINE_LOOP);
//        glVertex3f( sze,  0.0, 100.0);
//        glVertex3f( 0.0,  sze, 100.0);
//        glVertex3f(-sze,  0.0, 100.0);
//        glVertex3f( 0.0, -sze, 100.0);
//        glEnd();
//        glLineWidth(1.0);
//    }
    glColor3f(0, 1, 0);
    for (auto e: pList) {
        e->draw();
    }
    glLineWidth(1.0);
}




IAToolpathElement::IAToolpathElement()
{
}


IAToolpathElement::~IAToolpathElement()
{
}


void IAToolpathElement::draw()
{
    // nothing to here
}






IAToolpathMotion::IAToolpathMotion(IAVector3d &a, IAVector3d &b)
:   IAToolpathElement(),
    pStart(a),
    pEnd(b)
{
}


void IAToolpathMotion::draw()
{
    glBegin(GL_LINES);
//    glVertex3f(0, 0, 200);
//    glVertex3f(pStart.x(), pStart.y(), pStart.z());
//    glVertex3f(pEnd.x(), pEnd.y(), pEnd.z());
    glVertex3dv(pStart.dataPointer());
    glVertex3dv(pEnd.dataPointer());
    glEnd();
}




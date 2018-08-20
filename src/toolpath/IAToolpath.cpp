//
//  IAToolpath.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAToolpath.h"

#include "Iota.h"

#include <FL/gl.h>

#include <math.h>


/**
 * Create a list of toolpaths for the entire printout.
 */
IAMachineToolpath::IAMachineToolpath()
{
}


/**
 * Free all allocations.
 */
IAMachineToolpath::~IAMachineToolpath()
{
    clear();
}


/**
 * Free all allocations.
 */
void IAMachineToolpath::clear()
{
    delete pStartupPath;
    pStartupPath = nullptr;
    for (auto p: pLayerMap) {
        delete p.second;
    }
    pLayerMap.clear();
    delete pShutdownPath;
    pShutdownPath = nullptr;
}


/**
 * Draw the toolpath into the scene at world coordinates.
 */
void IAMachineToolpath::draw()
{
    pStartupPath->draw();
    for (auto p: pLayerMap) {
        p.second->draw();
    }
    pShutdownPath->draw();
}


/**
 * DRaw the toolpath of only one layer.
 */
void IAMachineToolpath::drawLayer(double z)
{
    auto p = findLayer(z);
    if (p)
        p->draw();
}


/**
 * Return a layer at the give z height, or nullptr if none found.
 */
IAToolpath *IAMachineToolpath::findLayer(double z)
{
    int layer = roundLayerNumber(z);
    auto p = pLayerMap.find(layer);
    if (p==pLayerMap.end())
        return nullptr;
    else
        return p->second;
}


/**
 * Create a new toolpath for a layer at the give z height.
 */
IAToolpath *IAMachineToolpath::createLayer(double z)
{
    int layer = roundLayerNumber(z);
    auto p = pLayerMap.find(layer);
    if (p==pLayerMap.end())
        return pLayerMap[layer];
    else
        return p->second;
}


/**
 * Delete a toolpath at the give heigt.
 */
void IAMachineToolpath::deleteLayer(double z)
{
    int layer = roundLayerNumber(z);
    pLayerMap.erase(layer);
}


/**
 * Round the z height into a layer number to avoid imprecissions of floating
 * point math.
 */
int IAMachineToolpath::roundLayerNumber(double z)
{
    return (int)lround(z*1000.0);
}




/**
 * Manage a single toolpath.
 */
IAToolpath::IAToolpath()
{
}


/**
 * Delete a toolpath.
 */
IAToolpath::~IAToolpath()
{
    clear();
}


/**
 * Clear a toolpath for its next use.
 */
void IAToolpath::clear()
{
    for (auto e: pList) {
        delete e;
    }
    pList.clear();
}


/**
 * Draw the current toolpath into the scene viewer at world coordinates.
 */
void IAToolpath::draw()
{
    glLineWidth(5.0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0, 1, 0);
    for (auto e: pList) {
        e->draw();
    }
    glLineWidth(1.0);
}


/**
 * Start a new path.
 */
void IAToolpath::startPath(double x, double y, double z)
{
    tFirst.set(x, y, z); tPrev = tFirst;
}


/**
 * Add a motion segment to the path.
 */
void IAToolpath::continuePath(double x, double y, double z)
{
    IAVector3d next(x, y, z);
    pList.push_back(new IAToolpathMotion(tPrev, next));
    tPrev = next;
}


/**
 * Create a loop by moving back to the very first vector.
 */
void IAToolpath::closePath()
{
    pList.push_back(new IAToolpathMotion(tPrev, tFirst));
}




/**
 * Create any sort of toolpath element.
 */
IAToolpathElement::IAToolpathElement()
{
}


/**
 * Destroy an element.
 */
IAToolpathElement::~IAToolpathElement()
{
}


/**
 * Draw any element.
 */
void IAToolpathElement::draw()
{
    // nothing to here
}



/**
 * Create a toolpath for a head motion to a new position.
 */
IAToolpathMotion::IAToolpathMotion(IAVector3d &a, IAVector3d &b)
:   IAToolpathElement(),
    pStart(a),
    pEnd(b)
{
}


/**
 * Draw the toolpath motion into the scene viewer.
 */
void IAToolpathMotion::draw()
{
    glBegin(GL_LINES);
    glVertex3dv(pStart.dataPointer());
    glVertex3dv(pEnd.dataPointer());
    glEnd();
}




//
//  IAToolpath.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#define RENDER_HEX_TOOLPATH


#include "IAToolpath.h"

#include "Iota.h"

#include <FL/gl.h>

#include <math.h>


bool isBlack(uint8_t *rgb, IAVector3d v)
{
    IAVector3d s = v * (kFramebufferSize / 214.0);
    int xo = (int)s.x(), yo = (int)s.y();
    uint8_t *c = rgb + (xo+kFramebufferSize*yo)*3;
    if (c[0]<128 && c[1]<128 && c[2]<128) {
        return true;
    } else {
        return false;
    }
}

/**
 * This hack slices long Toolpath Motions into smaller pieces if the color
 * of the object changes during that motion.
 */
void IAToolpath::colorize(uint8_t *rgb, IAToolpath *black, IAToolpath *white)
{
    for (auto e: pList) {
        IAToolpathMotion *m = dynamic_cast<IAToolpathMotion*>(e);
        if (m) {
            if (!m->pIsRapid) {
#if 0 // simple way to find color by looking at the start point
                if (isBlack(rgb, m->pStart)) {
                    black->pList.push_back(m->clone());
                } else {
                    white->pList.push_back(m->clone());
                }
#else // look at the vector and check the point color at every milimeter, splicing motion if needed
                IAVector3d startVec = m->pStart;
                IAVector3d currStartVec = startVec;
                IAVector3d currVec = startVec;
                IAVector3d endVec = m->pEnd;
                IAVector3d deltaVec = endVec - startVec;
                double len = (endVec-startVec).length();
                double incr = 0.1;
                bool color = isBlack(rgb, startVec);
                for (double i=incr; i<len; i+=incr) {
                    currVec = startVec + (deltaVec*(i/len));
                    bool colorNow = isBlack(rgb, currVec);
                    if (colorNow!=color) {
                        IAToolpathMotion *mtn = new IAToolpathMotion(currStartVec, currVec);
                        if (color) { mtn->pColor = 0x444444; black->pList.push_back(mtn); }
                        else { mtn->pColor = 0xFFFFFF; white->pList.push_back(mtn); }
                        currStartVec = currVec;
                        color = colorNow;
                    }
                }
                if (currStartVec!=endVec) {
                    IAToolpathMotion *mtn = new IAToolpathMotion(currStartVec, endVec);
                    if (color) { mtn->pColor = 0x444444; black->pList.push_back(mtn); }
                    else { mtn->pColor = 0xFFFFFF; white->pList.push_back(mtn); }
                }
#endif
            } else {

            }
        }
    }
}


uint32_t getRGB(uint8_t *rgb, IAVector3d v)
{
    IAVector3d s = v * (kFramebufferSize / 214.0);
    int xo = (int)s.x(), yo = (int)s.y();
    uint8_t *c = rgb + (xo+kFramebufferSize*yo)*3;
    return ((c[0]<<16)|(c[1]<<8)|(c[2]));
}

bool differ(uint32_t c1, uint32_t c2)
{
    if (c1==c2) return false;
    int r1 = (c1>>16)&255, r2 = (c2>>16)&255, rd = r1-r2;
    if (rd>10 || rd<-10) return true;
    int g1 = (c1>>8)&255, g2 = (c2>>8)&255, gd = g1-g2;
    if (gd>10 || gd<-10) return true;
    int b1 = (c1>>0)&255, b2 = (c2>>0)&255, bd = b1-b2;
    if (bd>10 || bd<-10) return true;
    return false;
}


void IAToolpath::colorizeSoft(uint8_t *rgb, IAToolpath *dst)
{
    for (auto e: pList) {
        IAToolpathMotion *m = dynamic_cast<IAToolpathMotion*>(e);
        if (m) {
            if (!m->pIsRapid) {
                IAVector3d startVec = m->pStart;
                IAVector3d currStartVec = startVec;
                IAVector3d currVec = startVec;
                IAVector3d endVec = m->pEnd;
                IAVector3d deltaVec = endVec - startVec;
                double len = (endVec-startVec).length();
                double incr = 0.1;
                uint32_t color = getRGB(rgb, startVec);
                for (double i=incr; i<len; i+=incr) {
                    currVec = startVec + (deltaVec*(i/len));
                    uint32_t colorNow = getRGB(rgb, currVec);
                    if (differ(colorNow, color)) {
                        IAToolpathMotion *mtn = new IAToolpathMotion(currStartVec, currVec);
                        mtn->setColor(color);
                        dst->pList.push_back(mtn);
                        currStartVec = currVec;
                        color = colorNow;
                    }
                }
                if (currStartVec!=endVec) {
                    IAToolpathMotion *mtn = new IAToolpathMotion(currStartVec, endVec);
                    mtn->setColor(color);
                    dst->pList.push_back(mtn);
                }
            } else {

            }
        }
    }
}





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
    if (pStartupPath) pStartupPath->draw();
    for (auto p: pLayerMap) {
        p.second->draw();
    }
    if (pShutdownPath) pShutdownPath->draw();
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
    if (p==pLayerMap.end()) {
        IAToolpath *tp = new IAToolpath(z);
        pLayerMap.insert(std::make_pair(layer, tp));
        return tp;
    } else {
        return p->second;
    }
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
 * Save the toolpath as a GCode file.
 */
bool IAMachineToolpath::saveGCode(const char *filename /*, printer */)
{
    bool ret = false;
    IAGcodeWriter w;
    if (w.open(filename)) {
        w.macroInit();
        if (pStartupPath)
            pStartupPath->saveGCode(w);
        for (auto p: pLayerMap) {
            w.cmdComment("");
            w.cmdComment("==== layer at z=%g", p.first / 1000.0);
            w.cmdComment("");
            w.cmdResetExtruder();
            // send all motion commands
            p.second->saveGCode(w);
        }
        if (pShutdownPath)
            pShutdownPath->saveGCode(w);
        w.macroShutdown();
        w.close();
        ret = true;
    }
    return ret;
}



int tpCount = 0;

/**
 * Manage a single toolpath.
 */
IAToolpath::IAToolpath(double z)
:   tFirst( 0.0, 0.0, z ),
    tPrev( 0.0, 0.0, z )
{
//    printf("Allocating toolpath (%d)\n", ++tpCount);
}


/**
 * Delete a toolpath.
 */
IAToolpath::~IAToolpath()
{
//    printf("Freeing toolpath (%d)\n", --tpCount);
    clear(0.0);
}


/**
 * Clear a toolpath for its next use.
 */
void IAToolpath::clear(double z)
{
    pZ = z;
    for (auto e: pList) {
        delete e;
    }
    pList.clear();
    tFirst = { 0.0, 0.0, z };
    tPrev = { 0.0, 0.0, z };
}


void IAToolpath::add(IAToolpath *tp)
{
    for (auto e: tp->pList) {
        pList.push_back(e->clone());
    }
}


/**
 * Draw the current toolpath into the scene viewer at world coordinates.
 */
void IAToolpath::draw()
{
#ifdef RENDER_HEX_TOOLPATH
    glLineWidth(1.0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glColor3f(0, 1, 0);
    for (auto e: pList) {
        e->draw();
    }
    glLineWidth(1.0);
#else
    glLineWidth(5.0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0, 1, 0);
    for (auto e: pList) {
        e->draw();
    }
    glLineWidth(1.0);
#endif
}


void IAToolpath::drawFlat(double w)
{
//    // Hack!
//    double scale = kFramebufferSize/Iota.pCurrentPrinter->pBuildVolume.x();
//    glLineWidth(w*scale);
//    glDisable(GL_TEXTURE_2D);
//    glDisable(GL_LIGHTING);
    /**
     \todo draw using polygons.
     \todo draw connection between lines.
     */
    for (auto e: pList) {
        e->drawFlat(w);
    }
//    glLineWidth(1.0);
}

/**
 * Start a new path.
 */
void IAToolpath::startPath(double x, double y, double z)
{
    IAVector3d next(x, y, z);
    tFirst = next;
    pList.push_back(new IAToolpathMotion(tPrev, next, true));
    tPrev = next;
}


/**
 * Add a motion segment to the path.
 */
void IAToolpath::continuePath(double x, double y, double z)
{
    IAVector3d next(x, y, z);
    if (!(tPrev==next))
        pList.push_back(new IAToolpathMotion(tPrev, next));
    tPrev = next;
}


/**
 * Create a loop by moving back to the very first vector.
 */
void IAToolpath::closePath()
{
    if (!(tPrev==tFirst))
        pList.push_back(new IAToolpathMotion(tPrev, tFirst));
}


/**
 * Save the toolpath as a GCode file.
 */
void IAToolpath::saveGCode(IAGcodeWriter &w)
{
    w.cmdComment("Send generated toolpath...");
    for (auto p: pList) {
        p->saveGCode(w);
    }
}


/**
 * Save the toolpath as a DXF file.
 */
void IAToolpath::saveDXF(const char *filename)
{
    IADxfWriter w;
    if (w.open(filename)) {
        for (auto p: pList) {
            p->saveDXF(w);
        }
        w.close();
    }
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


IAToolpathElement *IAToolpathElement::clone()
{
    return new IAToolpathElement();
}



/**
 * Draw any element.
 */
void IAToolpathElement::draw()
{
    // nothing to here
}



/**
 * Create any sort of toolpath element.
 */
IAToolpathExtruder::IAToolpathExtruder(int tool)
:   pTool( tool )
{
}


/**
 * Destroy an element.
 */
IAToolpathExtruder::~IAToolpathExtruder()
{
}


IAToolpathElement *IAToolpathExtruder::clone()
{
    IAToolpathExtruder *tpe = new IAToolpathExtruder(pTool);
    return tpe;
}


/**
 * Save the toolpath element as a GCode file.
 */
void IAToolpathExtruder::saveGCode(IAGcodeWriter &w)
{
    w.cmdComment("");
    w.cmdComment("---- Change to extruder %d", pTool);
    // deactivate the other extruder
    int otherTool = 1-pTool;
    w.cmdSelectExtruder(otherTool);
    w.cmdResetExtruder();
    w.cmdExtrude(-4.0);

    // activate the new extruder
    w.cmdSelectExtruder(pTool);
    w.cmdResetExtruder();
    w.cmdExtrude(4.0);
    int x = pTool ? 100 : 48;
    int pw = 20;
    w.cmdRapidMove(x, 10.0);
    int i;
    for (i=0; i<4; i++) {
        w.cmdMove(x+pw, 10.0+i);
        w.cmdMove(x+pw, 10.0+i+0.5);
        w.cmdMove(x, 10.0+i+0.5);
        w.cmdMove(x, 10.0+i+1.0);
    }
    w.cmdSelectExtruder(pTool); // redundant
    w.cmdResetExtruder();
    w.cmdComment("Extruder %d ready", pTool);
    w.cmdComment("");
}



/**
 * Create a toolpath for a head motion to a new position.
 */
IAToolpathMotion::IAToolpathMotion(IAVector3d &a, IAVector3d &b, bool rapid)
:   IAToolpathElement(),
    pStart( a ),
    pEnd( b ),
    pIsRapid( rapid )
{
}


IAToolpathElement *IAToolpathMotion::clone()
{
    IAToolpathMotion *mtn = new IAToolpathMotion(pStart, pEnd, pIsRapid);
    mtn->pColor = pColor;
    return mtn;
}


/**
 * Draw the toolpath motion into the scene viewer.
 *
 * \todo make the extrusion hexagonal so we can represent the squashing
 *       by the layer height. Also, use the current E factor to calculate the
 *       expected width of the extrusion and draw that.
 * \todo add lids or connecotrs to the next extrusion.
 * \todo this should be cached
 */
void IAToolpathMotion::draw()
{
#ifdef RENDER_HEX_TOOLPATH
    if (pIsRapid) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0, 1.0, 0.0);
        glEnable(GL_LIGHTING);
    } else {
        double r=0.2;
        IAVector3d d = (pEnd - pStart).normalized();
        IAVector3d n0 = { d.y(), -d.x(), 0.0 };
        IAVector3d n1 = { 0.0, 0.0, 1.0 };
        IAVector3d n2 = { -d.y(), d.x(), 0.0 };
        IAVector3d n3 = { 0.0, 0.0, -1.0 };
        IAVector3d p0, p1, p2, p3;

        glColor3ub(pColor>>16, pColor>>8, pColor);

        glBegin(GL_QUADS);
        glNormal3dv(n0.dataPointer());
        p0 = pStart + n0*r; glVertex3dv(p0.dataPointer());
        glNormal3dv(n0.dataPointer());
        p1 = pEnd + n0*r; glVertex3dv(p1.dataPointer());
        glNormal3dv(n1.dataPointer());
        p2 = pEnd + n1*r; glVertex3dv(p2.dataPointer());
        glNormal3dv(n1.dataPointer());
        p3 = pStart + n1*r; glVertex3dv(p3.dataPointer());
        glEnd();

        glBegin(GL_QUADS);
        glNormal3dv(n1.dataPointer());
        p0 = pStart + n1*r; glVertex3dv(p0.dataPointer());
        glNormal3dv(n1.dataPointer());
        p1 = pEnd + n1*r; glVertex3dv(p1.dataPointer());
        glNormal3dv(n2.dataPointer());
        p2 = pEnd + n2*r; glVertex3dv(p2.dataPointer());
        glNormal3dv(n2.dataPointer());
        p3 = pStart + n2*r; glVertex3dv(p3.dataPointer());
        glEnd();

        glBegin(GL_QUADS);
        glNormal3dv(n2.dataPointer());
        p0 = pStart + n2*r; glVertex3dv(p0.dataPointer());
        glNormal3dv(n2.dataPointer());
        p1 = pEnd + n2*r; glVertex3dv(p1.dataPointer());
        glNormal3dv(n3.dataPointer());
        p2 = pEnd + n3*r; glVertex3dv(p2.dataPointer());
        glNormal3dv(n3.dataPointer());
        p3 = pStart + n3*r; glVertex3dv(p3.dataPointer());
        glEnd();

        glBegin(GL_QUADS);
        glNormal3dv(n3.dataPointer());
        p0 = pStart + n3*r; glVertex3dv(p0.dataPointer());
        glNormal3dv(n3.dataPointer());
        p1 = pEnd + n3*r; glVertex3dv(p1.dataPointer());
        glNormal3dv(n0.dataPointer());
        p2 = pEnd + n0*r; glVertex3dv(p2.dataPointer());
        glNormal3dv(n0.dataPointer());
        p3 = pStart + n0*r; glVertex3dv(p3.dataPointer());
        glEnd();

    }
#else
    if (pIsRapid) {
        glLineWidth(1.0);
        glColor3f(1.0, 1.0, 0.0);
    } else {
        glLineWidth(2.0);
        glColor3f(1.0, 0.0, 1.0);
    }
    glBegin(GL_LINES);
    glVertex3dv(pStart.dataPointer());
    glVertex3dv(pEnd.dataPointer());
    glEnd();
    glLineWidth(1.0);
#endif
}


/**
 * Draw the toolpath motion into the scene viewer.
 */
void IAToolpathMotion::drawFlat(double w)
{
    if (!pIsRapid) {
#if 1
        /**
         \todo this should draw a cap depending on the previous line.
         \todo this is the brute force approach which could be made so much
            faster. This approach just draws an octagon, extende by a line.
         */
        IAVector3d d = pEnd - pStart;
        IAVector3d u = d.normalized();
        double xo = u.x() * w * 0.5, x7 = xo * 0.7;
        double yo = u.y() * w * 0.5, y7 = yo * 0.7;;
        glBegin(GL_POLYGON);
        glVertex3d(pStart.x()-xo, pStart.y()-yo, pStart.z());
        glVertex3d(pStart.x()-x7-y7, pStart.y()-y7+x7, pStart.z());
        glVertex3d(pStart.x()-yo, pStart.y()+xo, pStart.z());
        glVertex3d(pEnd.x()-yo, pEnd.y()+xo, pEnd.z());
        glVertex3d(pEnd.x()+x7-y7, pEnd.y()+y7+x7, pEnd.z());
        glVertex3d(pEnd.x()+xo, pEnd.y()+yo, pEnd.z());
        glVertex3d(pEnd.x()+x7+y7, pEnd.y()+y7-x7, pEnd.z());
        glVertex3d(pEnd.x()+yo, pEnd.y()-xo, pEnd.z());
        glVertex3d(pStart.x()+yo, pStart.y()-xo, pStart.z());
        glVertex3d(pStart.x()-x7+y7, pStart.y()-y7-x7, pStart.z());
        glEnd();
#else
        // FIXME: line width!
        glBegin(GL_LINES);
        glVertex3dv(pStart.dataPointer());
        glVertex3dv(pEnd.dataPointer());
        glEnd();
#endif
    }
}


/**
 * Save the toolpath element as a GCode file.
 */
void IAToolpathMotion::saveGCode(IAGcodeWriter &w)
{
#ifdef IA_QUAD
    if (pIsRapid) {
        w.cmdRetract();
        w.cmdRapidMove(pEnd);
        w.cmdUnretract();
    } else {
        if (w.position()!=pStart) {
            w.cmdRetract();
            w.cmdRapidMove(pStart);
            w.cmdUnretract();
        }
        w.cmdMove(pEnd, pColor);
    }
#else
    if (pIsRapid) {
        w.cmdRetract();
        w.cmdRapidMove(pEnd);
        w.cmdUnretract();
    } else {
        if (w.position()!=pStart) {
            w.cmdRetract();
            w.cmdRapidMove(pStart);
            w.cmdUnretract();
        }
        w.cmdMove(pEnd);
    }
#endif
}


/**
 * Save the toolpath element as line in a DXF file.
 */
void IAToolpathMotion::saveDXF(IADxfWriter &g)
{
    if (!pIsRapid) {
        g.cmdLine(pStart, pEnd);
    }
}




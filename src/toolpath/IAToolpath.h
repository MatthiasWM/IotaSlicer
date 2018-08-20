//
//  IAToolpath.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_TOOLPATH_H
#define IA_TOOLPATH_H


#include "../geometry/IAVector3d.h"

#include <vector>
#include <map>


class IAToolpath;
class IAToolpathElement;

typedef std::map<double, IAToolpath*> IAToolpathMap;
typedef std::vector<IAToolpathElement*> IAToolpathElementList;


/**
 * Represents a list commands (settings, motions) for a 3d printer.
 *
 * This is an internal storage format for commands that need to be sent
 * to a machine to create the desired 3D printout.
 *
 * This class is used to draw the toolpath into the scene view. It is also
 * the source for generating GCode commands. All coordinates in this class
 * must be kept in world space. The GCode writer must flip or offset
 * coordinates to math the printer space. Scaling should not be needed.
 */
class IAMachineToolpath
{
public:
    IAMachineToolpath();
    ~IAMachineToolpath();

    void clear();
    void draw();
    void drawLayer(double);

    IAToolpath *pStartupPath = nullptr;
    IAToolpathMap pLayerMap;
    IAToolpath *pShutdownPath = nullptr;
};


/**
 * A toolpath, usually for a single slice, for startup, or for shutdown.
 */
class IAToolpath
{
public:
    IAToolpath();
    ~IAToolpath();
    void clear();
    void draw();

    void startPath(double x, double y, double z);
    void continuePath(double x, double y, double z);
    void closePath(void);

    IAToolpathElementList pList;
    // list of elements

    IAVector3d tFirst, tPrev;

};


/**
 * Base class for any element inside the tool path.
 */
class IAToolpathElement
{
public:
    IAToolpathElement();
    virtual ~IAToolpathElement();
    virtual void draw();
};


/**
 * Toolpath element for extruder motion.
 */
class IAToolpathMotion : public IAToolpathElement
{
public:
    IAToolpathMotion(IAVector3d &a, IAVector3d &b);
    virtual void draw();
    // FIXME: we MUST NOT have start and end. The previous end and the current
    // start are redundant and caus trouble if assumptions are made!
    IAVector3d pStart, pEnd;
    bool pIsRapid = false;
};


/*
 further elements could be
  - tool change
  - tool cleanin
  - machine setup, bed heating, extruder heating, etc.
 */


#endif /* IA_TOOLPATH_H */


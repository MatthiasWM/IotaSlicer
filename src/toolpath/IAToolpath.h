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

typedef std::vector<IAToolpathElement*> IAToolpathElementList;


class IAMachineToolpath
{
public:
    IAMachineToolpath();
    // startup path
    // list of layer paths
    // shutdown path
};


/**
 * Represents a list commands (settings, motions) for a 3d printer.
 *
 * This is an internal storage format for commands that need to be sent
 * to a machine to create the desired 3D printout.
 *
 * This class is used to draw the toolpath into the scene view. It is also
 * the source for generating GCode commands.
 */
class IAToolpath
{
public:
    IAToolpath();
    ~IAToolpath();
    void clear();
    void draw();
    IAToolpathElementList pList;
    // list of elements
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



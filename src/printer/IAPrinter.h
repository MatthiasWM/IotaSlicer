//
//  IAPrinter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_H
#define IA_PRINTER_H


#include "geometry/IAVector3d.h"


/**
 Manage different types of 3D printers.
 */
class IAPrinter
{
public:
    IAPrinter();
    void draw();

    IAVector3d pBuildVolume = { 214.0, 214.0, 230.0 };
    IAVector3d pBuildVolumeMin = { 0.0, 0.0, 0.0 };
    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 230.0 };
    double pBuildVolumeRadius = 200.0; // sphere that contains the entire centered build volume

};


#endif /* IA_PRINTER_H */



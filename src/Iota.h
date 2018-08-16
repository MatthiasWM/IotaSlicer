//
//  Iota.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "geometry/IAMesh.h"
#include "geometry/IASlice.h"
#include "printer/IAPrinter.h"

// TODO globals that we want to get rid of.

const int IA_PROJECTION_FRONT       = 0;
const int IA_PROJECTION_CYLINDER    = 1;
const int IA_PROJECTION_SPHERE      = 2;

extern double min(double a, double b);
extern double max(double a, double b);
extern float min(float a, float b);
extern float max(float a, float b);

/**
 * The main class that manages this app.
 */
class IAIota
{
public:
    IAIota();
    ~IAIota();

    bool addGeomtery(const char *name, uint8_t *data, size_t size);
    bool addGeomtery(const char *filename);
    void addGeometry(IAMeshList *model);

private:
    bool addGeomtery(class IAGeometryReader *reader);


public: // TODO: deprecated globals are now members, but must be removed
    class Fl_Window *gMainWindow = nullptr;
    class Fl_RGB_Image *texture = nullptr;
    IAMeshList *gMeshList = nullptr;
    IASlice gMeshSlice;
    IAPrinter gPrinter;
    bool gShowSlice;
    bool gShowTexture;
    FILE *gOutFile = nullptr;
    double minX, maxX, minY, maxY, minZ, maxZ;

    void sliceAll();
    void menuWriteSlice();
    void menuQuit();
};


extern IAIota Iota;


#endif /* MAIN_H */

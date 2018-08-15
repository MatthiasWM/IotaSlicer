//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

// TODO: add drag'n'drop of STL files to IAModelView
// TODO: fix STL importer to generate only watertight models
// TODO: create a model class that contains meshes
// TODO: position new models in the center and drop them on the build plane
// TODO: render textures as slices in IAModelView
// TODO: prototyped - generate slices as OpenGL Textures
// TODO: prototyped - write slices to disk as images
// TODO: prototyped - create vector outline from slices
// TODO: shrink slices
// TODO: create vector infills from slices
// TODO: prototyped - create GCode from vectors
// Done (LOL)

// TODO: port to Linux

#include "main.h"

#include "data/binaryData.h"
#include "userinterface/mainUI.h"
#include "fileformats/IAFmtTexJpeg.h"
#include "fileformats/IAFmtObj3ds.h"
#include "fileformats/IAFmtObjStl.h"

Fl_Window *gMainWindow = nullptr;
Fl_RGB_Image *texture = 0L;

IAMeshList gMeshList;
IASlice gMeshSlice;
IAPrinter gPrinter; // Allocate default printer

bool gShowSlice = false;
bool gShowTexture = false;
FILE *gOutFile;

double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;


float min(float a, float b) { return (a<b)?a:b; }

float max(float a, float b) { return (a>b)?a:b; }

double min(double a, double b) { return a<b?a:b; }

double max(double a, double b) { return a>b?a:b; }


/**
 Experimental stuff.
 */
void menuWriteSlice()
{
    char buf[FL_PATH_MAX];
    sprintf(buf, "%s/slice.jpg", getenv("HOME"));
    gMeshSlice.save(zSlider1->value(), buf);
}

void menuQuit()
{
    gMainWindow->hide();
    Fl::flush();
    exit(0);
}

void sliceAll()
{
//    gMeshSlice.generateLidFrom(gMeshList, zSlider1->value());
//    defer slicing until we actually need a to recreate the lid
}




int main (int argc, char **argv)
{
    // TODO: remember the window position and size in the preferences

    Fl::use_high_res_GL(1);

    gMainWindow = createIotaAppWindow();
    gMainWindow->show(argc, argv);
    Fl::flush();

    loadTexture("testcard1024.jpg", defaultTexture);
//    loadStl("/Users/matt/dev/IotaSlicer/data/xyz.stl");
//    loadStl("/Users/matt/dev/IotaSlicer/src/data/suzanne.stl");
    loadStl(defaultModel);
    gMeshList.projectTexture(100.0, 100.0, IA_PROJECTION_FRONT);

    glView->redraw();

    return Fl::run();
}

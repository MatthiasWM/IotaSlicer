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
// TODO: generate slices as OpenGL Textures
// TODO: write slices to disk as images
// TODO: create vector outline from slices
// TODO: shring slices
// TODO: create vector infills from slices
// TODO: create GCode from vectors
// Done (LOL)

// TODO: port to Linux and MSWindows

#include "main.h"

#include "binaryData.h"
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
FILE *gOutFile;

double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;


float min(float a, float b) { return (a<b)?a:b; }

float max(float a, float b) { return (a>b)?a:b; }

double min(double a, double b) { return a<b?a:b; }

double max(double a, double b) { return a>b?a:b; }


void menuQuit()
{
    gMainWindow->hide();
    Fl::flush();
    exit(0);
}

void sliceAll()
{
    gMeshSlice.generateFrom(gMeshList, zSlider1->value());
    // start a new slice. A slice holds the information from all meshes.
    gMeshSlice.clear();
    // get the number of meshes in this model
    int i, n = (int)gMeshList.size();
    // loop through all meshes
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = gMeshList[i];
        // add all faces in a mesh that intersect with zMin. They will form the lower lid.
        gMeshSlice.addZSlice(*IAMesh, zSlider1->value());
        // use OpenGL to convert the sorted list of edges into a list of simple polygons
        gMeshSlice.tesselate();
    }
    glView->redraw();
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

    glView->redraw();

    return Fl::run();
}

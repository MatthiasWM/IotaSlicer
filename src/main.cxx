//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

//#define M_MONKEY
//#define M_DRAGON
#define M_XYZ

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
#ifdef M_MONKEY
    load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/lib3ds-20080909/monkey.3ds");
    //loadStl(defaultModel);
#elif defined M_DRAGON
    loadStl("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
#elif defined M_XYZ
    //    loadStl("/Users/matt/dev/IotaSlicer/data/xyz.stl");
    loadStl(defaultModel);
#endif

    glView->redraw();

    return Fl::run();
}

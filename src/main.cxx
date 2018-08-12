//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

//#define M_MONKEY
//#define M_DRAGON
#define M_XYZ

#include "binaryData.h"
#include "userinterface/IAModelView.h"
#include "fileformats/IAFmtTexJpeg.h"
#include "fileformats/IAFmtObj3ds.h"
#include "fileformats/IAFmtObjStl.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/gl.h>
#include <FL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "lib3ds.h"

#include "IAMesh.h"
#include "IASlice.h"

#include "printer/IAPrinter.h"




#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif

Fl_Slider *zSlider1, *zSlider2;
Fl_RGB_Image *texture = 0L;

IAMeshList gMeshList;
IASlice gMeshSlice;
IAPrinter gPrinter; // Allocate default printer

GLUtesselator *gGluTess = 0;

bool gShowSlice = false;
int gWriteSliceNext = 0;

FILE *gOutFile;

// -----------------------------------------------------------------------------

void writeInt(FILE *f, int32_t x)
{
    uint8_t v;
    // bits 34..28
    if (x&(0xffffffff<<28)) {
        v = ((x>>28) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 27..21
    if (x&(0xffffffff<<21)) {
        v = ((x>>21) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 20..14
    if (x&(0xffffffff<<14)) {
        v = ((x>>14) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 13..7
    if (x&(0xffffffff<<7)) {
        v = ((x>>7) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 6..0
    v = x & 0x7f;
    fputc(v, f);
}

// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------

float min(float a, float b) { return (a<b)?a:b; }
float max(float a, float b) { return (a>b)?a:b; }

double min(double a, double b) { return a<b?a:b; }
double max(double a, double b) { return a>b?a:b; }


#if 0
void setShaders() {

    GLuint v, f, p;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    /*
     vec4 v = vec4(gl_Vertex);
     v.z = 0.0;
     gl_Position = gl_ModelViewProjectionMatrix * v;
     */
    const char * vv =
    "void main(void) {\n"
    "    gl_Position = ftransform();\n"
    "}";

    //  "void main()\n"
    //  "{\n"
    //  "  gl_Position = ftransform();\n"
    //  "}\n"
    //  ;

    // if (pixelIsSilly) dicard;
    const char * ff =
    "void main() {\n"
    "    gl_FragColor = vec4( 1, 1, 0, 1);\n"
    "}"
    ;

    glShaderSource(v, 1, &vv,NULL);
    glShaderSource(f, 1, &ff,NULL);

    glCompileShader(v);
    glCompileShader(f);

    p = glCreateProgram();

    glAttachShader(p,v);
    glAttachShader(p,f);

    glLinkProgram(p);
    glUseProgram(p);
}
#endif


IAModelView *glView = 0L;

double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;



static void xButtonCB(Fl_Widget*, void*)
{
    gShowSlice = false;
    glView->redraw();
}

static void sliceCB(Fl_Widget*, void*)
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

static void z1ChangedCB(Fl_Widget*, void*)
{
    sliceCB(0, 0);
    gShowSlice = true;
    glView->redraw();
}

static void z2ChangedCB(Fl_Widget*, void*)
{
    gShowSlice = true;
    glView->redraw();
}

static void writeSliceCB(Fl_Widget*, void*)
{
    double z;
#ifdef M_MONKEY
    double firstLayer  = -8.8;
    double lastLayer   =  9.0;
    double layerHeight =  0.1;
    gOutFile = fopen("/Users/matt/monkey.3dp", "wb");
#elif defined M_DRAGON
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
    gOutFile = fopen("/Users/matt/dragon.3dp", "wb");
#elif defined M_XYZ
    double firstLayer  =   0.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
    gOutFile = fopen("/Users/matt/pepsi.3dp", "wb");
#endif
    // header
    writeInt(gOutFile, 23);  // Magic
    writeInt(gOutFile, 3);
    writeInt(gOutFile, 2013);
    writeInt(gOutFile, 1);   // File Version
    writeInt(gOutFile, 159); // total number of layers
    writeInt(gOutFile,  (lastLayer-firstLayer)/layerHeight );

    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
        // spread powder
        writeInt(gOutFile, 158);
        writeInt(gOutFile,  10); // spread 0.1mm layers
        // render the layer
        zSlider1->value(z);
        zSlider1->do_callback();
        gWriteSliceNext = 1;
        glView->redraw();
        glView->flush();
        Fl::flush();
    }
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    fclose(gOutFile);
    fprintf(stderr, "/Users/matt/monkey.3dp");
}


static void writePrnSliceCB(Fl_Widget*, void*)
{
    double z;
#ifdef M_MONKEY
    double firstLayer  = -8.8;
    double lastLayer   =  9.0;
    double layerHeight =  0.1;
#elif defined M_DRAGON
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
#elif defined M_XYZ
    double firstLayer  =   0.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
#endif

    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
        zSlider1->value(z);
        zSlider1->do_callback();
        gWriteSliceNext = 2;
        glView->redraw();
        glView->flush();
        Fl::flush();
    }
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    fclose(gOutFile);
    fprintf(stderr, "/Users/matt/monkey.3dp");
}


int main (int argc, char **argv)
{
    /*
     500 pixles at 96dpi = 5in = 13cm
     at 96dpi, 1 dot is 0.26mm in diameter
     */
    loadTexture("testcard1024.jpg", defaultTexture);

    Fl::use_high_res_GL(1);
    Fl_Window win(840, 800, "Iota Slice");
    win.begin();
    Fl_Group *g = new Fl_Group(0, 0, 800, 800);
    g->begin();
    glView = new IAModelView(150, 150, 500, 500);
    g->end();
    zSlider1 = new Fl_Slider(800, 0, 20, 680);
    zSlider1->tooltip("Position of the slice in Z\n-66 to +66 millimeters");
    zSlider1->range(30, -30);
    zSlider1->step(0.25);
    zSlider1->callback(z1ChangedCB);
    zSlider2 = new Fl_Slider(820, 0, 20, 680);
    zSlider2->tooltip("Slice thickness\n-10 to +10 millimeters");
    zSlider2->range(10, -10);
    zSlider2->value(0.25);
    zSlider2->callback(z2ChangedCB);
    Fl_Button *b = new Fl_Button(800, 680, 40, 40, "X");
    b->callback(xButtonCB);
    b = new Fl_Button(800, 720, 40, 40, "Write");
    b->callback(writeSliceCB);
    b = new Fl_Button(800, 760, 40, 40, ".prn");
    b->callback(writePrnSliceCB);
    win.end();
    win.resizable(g);
    win.show(argc, argv);
    glView->show();
    Fl::flush();
#ifdef M_MONKEY
    load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/lib3ds-20080909/monkey.3ds");
    //loadStl(defaultModel);
#elif defined M_DRAGON
    loadStl("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
#elif defined M_XYZ
//    loadStl("/Users/matt/dev/IotaSlicer/data/xyz.stl");
    loadStl(defaultModel);
#endif
    //load3ds("/Users/matt/squirrel/NewSquirrel.3ds");
    //load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.3ds");
    glView->redraw();
    return Fl::run();
}

//
// End of "$Id: hello.cxx 11782 2016-06-14 11:15:22Z AlbrechtS $".
//


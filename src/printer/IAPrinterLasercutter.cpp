//
//  IAPrinterLasercutter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterLasercutter.h"

#include "Iota.h"
#include "view/IAGUIMain.h"
#include "view/IAProgressDialog.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"


#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>


IAPrinterLasercutter::IAPrinterLasercutter()
:   super()
{
    // nothing to initialize
}


IAPrinterLasercutter::IAPrinterLasercutter(IAPrinterLasercutter const& src)
:   super(src)
{
    // nothing to copy
}


IAPrinterLasercutter::~IAPrinterLasercutter()
{
    // nothing to delete
}


IAPrinter *IAPrinterLasercutter::clone() const
{
    IAPrinterLasercutter *p = new IAPrinterLasercutter(*this);
    return p;
}




/**
 * Save the current slice data to a prepared filename.
 *
 * Verify a given filename when this is the first call in a session. Request
 * a new filename if none was set yet.
 */
void IAPrinterLasercutter::userSliceSave()
{
    if (pFirstWrite) {
        userSliceSaveAs();
    } else {
        // FIXME: if not yet sliced, so it
        //sliceAll();
        // FIXME: save to disk
        saveToolpaths();
    }
}


/**
 * Implement this to open a file chooser with the require file
 * pattern and extension.
 */
void IAPrinterLasercutter::userSliceSaveAs()
{
    if (queryOutputFilename("Save toolpath as GCode", "*.gcode", ".gcode")) {
        pFirstWrite = false;
        userSliceSave();
    }
}


/**
 * Generate all slice data and cache it for a fast preview or save operation.
 */
void IAPrinterLasercutter::userSliceGenerateAll()
{
    purgeSlicesAndCaches();
    sliceAll();
}


/**
 * Create and cache all slices.
 */
void IAPrinterLasercutter::sliceAll()
{
    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = layerHeight() * 0.7;
    double zLayerHeight = layerHeight();
    double zMax = hgt;

    // show a dialog to give the user a feedback for the Build choice
    IAProgressDialog::show("Genrating slices",
                           "Slicing layer %d of %d at %.3fmm (%d%%)");
    Fl::wait(0.1);

    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);
    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        if (IAProgressDialog::update(i*100/n, i, n, z, i*100/n)) break;
#if 0 // we do not cache these types of slices yet
        gSlice.setNewZ(z);
        gSlice.clear();
        gSlice.generateRim(Iota.pMesh);
        gSlice.tesselateLidFromRim();
        gSlice.drawFlat(false, 1, 1, 1);
#endif
        i++;
    }
    IAProgressDialog::hide();
    gSceneView->redraw();
}


/**
 * Virtual, implement the slicer for the given machine here.
 */
void IAPrinterLasercutter::saveToolpaths(const char *filename)
{
    assert(0);
#if 0
    if (!filename)
        filename = outputPath();
    char fn[FL_PATH_MAX];
    strcpy(fn, filename);
    char *a = (char*)fl_filename_ext(fn);
    const char *b = fl_filename_ext(filename);
    if ( a && b ) {
        sprintf(a, "_%%04d%s", b);
    } else {
        strcat(fn, "_%04d");
    }

    pMachineToolpath.clear();
    
    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = 0.5 * layerHeight();
    double zLayerHeight = layerHeight();
    double zMax = hgt;

    IAProgressDialog::show("Saving slices",
                           "Writing layer %d of %d at %.3fmm (%d%%)");
    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);
    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        if (IAProgressDialog::update(i*100/n, i, n, z, i*100/n)) break;

        gSlice.setNewZ(z);
        gSlice.clear();
        gSlice.generateRim(Iota.pMesh);
        gSlice.tesselateLidFromRim();
        gSlice.drawFlat(false, 1, 1, 1);

        uint8_t *rgb = gSlice.pColorbuffer->getRawImageRGB();
        IAToolpathList *tp = new IAToolpathList(z);
        gSlice.pFramebuffer->traceOutline(tp, z);
//        tp->pToolpathList.push_back(new IAToolpathExtruder(1));
        //        FIXME: what does the call above do?

        char dxfFilename[2048];
        sprintf(dxfFilename, fn, i);
        tp->saveDXF(dxfFilename);

        delete tp;
        free(rgb);
        i++;
    }
    IAProgressDialog::hide();
    gSceneView->redraw();
#endif
}



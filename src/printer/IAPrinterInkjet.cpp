//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterInkjet.h"

#include "Iota.h"
#include "view/IAGUIMain.h"
#include "view/IAProgressDialog.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"


#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>



IAPrinterInkjet::IAPrinterInkjet()
:   super()
{
    // nothing to initialize
}


IAPrinterInkjet::IAPrinterInkjet(IAPrinterInkjet const& src)
:   super(src)
{
    // nothing to copy
}


IAPrinterInkjet::~IAPrinterInkjet()
{
    // nothing to delete
}


IAPrinter *IAPrinterInkjet::clone() const
{
    IAPrinterInkjet *p = new IAPrinterInkjet(*this);
    return p;
}



/**
 * Save the current slice data to a prepared filename.
 *
 * Verify a given filename when this is the first call in a session. Request
 * a new filename if none was set yet.
 */
void IAPrinterInkjet::userSliceSave()
{
    if (pFirstWrite) {
        userSliceSaveAs();
    } else {
        /** \bug if not yet sliced, do it */
        // sliceAll();
        /** \bug save to disk */
        saveSlices();
    }
}


/**
 * Implement this to open a file chooser with the require file
 * pattern and extension.
 */
void IAPrinterInkjet::userSliceSaveAs()
{
    if (queryOutputFilename("Save slices a png images", "*.png", ".png")) {
        pFirstWrite = false;
        userSliceSave();
    }
}


/**
 * Generate all slice data and cache it for a fast preview or save operation.
 */
void IAPrinterInkjet::userSliceGenerateAll()
{
    purgeSlicesAndCaches();
    sliceAll();
}


/**
 * Create and cache all slices.
 *
 * \todo the texture mapping currently projects only along the xy-normal, and
 *       not along the point normal as it should be.
 */
void IAPrinterInkjet::sliceAll()
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
 * Write previously generated slices to disk.
 *
 * If we had cached slices, we could write them now. We don;t do that yet,
 * so wemust generate slices on the fly.
 */
void IAPrinterInkjet::saveSlices(const char *filename)
{
    if (!filename)
        filename = recentUpload();
    char fn[FL_PATH_MAX+1];
    strcpy(fn, filename);
    char *a = (char*)fl_filename_ext(fn);
    const char *b = fl_filename_ext(filename);
    if ( a && b ) {
        snprintf(a, fn+FL_PATH_MAX-a, "_%%04d%s", b);
    } else {
        strncat(fn, "_%04d", FL_PATH_MAX);
    }

    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z() + 2.0*layerHeight();
    double zMin = layerHeight() * 0.9; // initial height
    double zLayerHeight = layerHeight();
    double zMax = hgt;

    IAProgressDialog::show("Writing slices",
                           "Writing layer %d of %d at %.3fmm (%d%%)");


    int i = 0, n = (int)((zMax-zMin)/zLayerHeight) + 2;

    for (i=0; i<n; ++i)
    {
        double z = i * layerHeight() + 0.5 /* + first layer offset */;
        if (IAProgressDialog::update(i*100/n, i, n, z, i*100/n)) break;

        gSlice.setNewZ(z);
        gSlice.clear();
        gSlice.generateRim( Iota.pMesh );
        gSlice.tesselateAndDrawLid(gSlice.pColorbuffer);
        uint8_t *rgb = gSlice.pColorbuffer->getRawImageRGBA();

        char imgFilename[2048];
        snprintf(imgFilename, 2047, fn, i);
        gSlice.pColorbuffer->saveAsPng(imgFilename, 4, rgb, true);
        // TODO: we should make the file format depend to the filename extension
        // for testing, we also can write jpegs or other files.
        //        fl_filename_setext(imgFilename, 2048, ".jpg");
        //        gSlice.pColorbuffer->saveAsJpeg(imgFilename, rgb);
        ::free(rgb);
    }

    IAProgressDialog::hide();
    gSceneView->redraw();
}



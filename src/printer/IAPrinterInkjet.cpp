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
        // FIXME: if not yet sliced, so it
        // sliceAll();
        // FIXME: save to disk
        saveSlices();
    }
}


/**
 * Implement this to open a file chooser with the require file
 * pattern and extension.
 */
void IAPrinterInkjet::userSliceSaveAs()
{
    if (queryOutputFilename("Save toolpath as GCode", "*.gcode", ".gcode")) {
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
    char fn[FL_PATH_MAX];
    strcpy(fn, filename);
    char *a = (char*)fl_filename_ext(fn);
    const char *b = fl_filename_ext(filename);
    if ( a && b ) {
        sprintf(a, "_%%04d%s", b);
    } else {
        strcat(fn, "_%04d");
    }

//    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

//    double zMin = layerHeight() * 0.7;
//    double zLayerHeight = layerHeight();
//    double zMax = hgt;

    IAProgressDialog::show("Writing slices",
                           "Writing layer %d of %d at %.3fmm (%d%%)");

//    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);
//    for (double z=zMin; z<zMax; z+=zLayerHeight) {
//        if (IAProgressDialog::update(i*100/n, i, n, z, i*100/n)) break;
//
//        gSlice.setNewZ(z);
//        gSlice.clear();
//        gSlice.generateRim(Iota.pMesh);
//        gSlice.tesselateLidFromRim();
//        gSlice.draw(IAMesh::kMASK, 1, 1, 1);
//
//        uint8_t *alpha = gSlice.pFramebuffer->getRawImageRGB();
//        uint8_t *rgb = gSlice.pColorbuffer->getRawImageRGBA();
//
//        /**
//         \todo we can of course do all that in the OpenGL code already
//         \todo infill should be white or user selectable
//         \todo inkjet should generate support structurs for image based SLA
//         */
//        {
//            int i = 0, n = gSlice.pColorbuffer->width()
//                         * gSlice.pColorbuffer->height();
//            uint8_t *s = alpha, *d = rgb;
//            for (i=0; i<n; ++i) {
//                d[3] = *s;
//                s+=3; d+=4;
//            }
//        }
//
//        char imgFilename[2048];
//        sprintf(imgFilename, fn, i);
//        gSlice.pColorbuffer->saveAsPng(imgFilename, 4, rgb);
//        // for testing, we also can write jpegs or other files.
//        //        fl_filename_setext(imgFilename, 2048, ".jpg");
//        //        gSlice.pColorbuffer->saveAsJpeg(imgFilename, rgb);
//        i++;
//    }
    IAProgressDialog::hide();
    gSceneView->redraw();
}



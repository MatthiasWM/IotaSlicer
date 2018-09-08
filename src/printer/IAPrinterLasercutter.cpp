//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterLasercutter.h"


//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterLasercutter.h"

#include "Iota.h"
#include "userinterface/IAGUIMain.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"


#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>


/**
 * Virtual, implement this to open a file chooser with the require file
 * pattern and extension.
 */
void IAPrinterLasercutter::userSliceAs()
{
    if (queryOutputFilename("Save slices as numbered DXF files", "*.dxf", ".dxf")) {
        sliceAndWrite();
    }
}


/**
 * Virtual, implement the slicer for the given machine here.
 */
void IAPrinterLasercutter::sliceAndWrite(const char *filename)
{
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

    if (!Iota.pMachineToolpath)
        Iota.pMachineToolpath = new IAMachineToolpath();
    else
        Iota.pMachineToolpath->clear();
    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = 0.5 * layerHeight();
    double zLayerHeight = layerHeight();
    double zMax = hgt;

    showProgressDialog();
    char buf[1024];
    strcpy(buf, "Genrating slices");
    wProgressText->copy_label(buf);
    wProgressValue->value(0);
    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);

    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        sprintf(buf, "Slicing layer %d of %d at %.3fmm (%d%%)", i, n, z, i*100/n);
        wProgressText->copy_label(buf);
        wProgressValue->value(i*100/n);
        bool abort = updateProgressDialog();
        if (abort) break;

        Iota.gMeshSlice.changeZ(z);
        Iota.gMeshSlice.clear();
        Iota.gMeshSlice.generateRim(Iota.pMesh);
        Iota.gMeshSlice.tesselateLidFromRim();
        Iota.gMeshSlice.drawFlat(false, 1, 1, 1);

        uint8_t *rgb = Iota.gMeshSlice.pColorbuffer->getRawImageRGB();
        IAToolpath *tp = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp, z);
        tp->pList.push_back(new IAToolpathExtruder(1));

        char dxfFilename[2048];
        sprintf(dxfFilename, fn, i);
        tp->saveDXF(dxfFilename);

        delete tp;
        free(rgb);
        i++;
    }
    hideProgressDialog();
    gSceneView->redraw();
}


/**
 * Create the Treeview items for setting up the printout for this session.
 */
void IAPrinterLasercutter::buildSessionSettings()
{
    char buf[80];

    static Fl_Menu_Item lHgtMenu[] = {
        { "1.0" },
        { "2.0" },
        { "3.0" },
        { "4.0" }
    };

    Fl_Input_Choice *lHgt = new Fl_Input_Choice(1, 1, 60, 1, "mm");
    lHgt->align(FL_ALIGN_RIGHT);
    lHgt->labelsize(12);
    lHgt->textsize(12);
    lHgt->menu(lHgtMenu);
    sprintf(buf, "%.2f", layerHeight()); lHgt->value(buf);
    lHgt->callback(userSetLayerHeightCB, this);
    Fl_Tree_Item *it = wSessionSettings->add("Layer Height: ");
    it->widget(lHgt);
}


/**
 * The layer height was changed via the layer height chooser in the seesion
 * setting view.
 */
void IAPrinterLasercutter::userSetLayerHeight(Fl_Input_Choice *w)
{
    /** \todo warn if layer height is 0, negative, or huge */
    pLayerHeight = atof(w->value());
}



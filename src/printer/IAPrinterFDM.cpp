//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterFDM.h"

#include "Iota.h"
#include "userinterface/IAGUIMain.h"
#include "userinterface/IAProgressDialog.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"


#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>


/*
 How do we find a lid?

 A single layer lid is the AND NOT operation between this layer and the
 layer above this one. Everything in this layer that is not the lid is
 then the infill.

 A multi layer lid is the AND NOT operation between this layer and
 the AND operation of multiple layers above this one.

 A bottom lid is the same as a top lib, but with the layers below. A
 general lid is then the current pattern AND NOT the AND operation of
 all relevant layers below or above.

 Again the remaining part is the infill, or, to put it more
 mathematically, the infill is the AND operation of all layers
 involved.
 */

/*
 How do we find the support structure pattern?

 There are two supports needed: triangles that are flatter than 45 deg
 from z need support, and "icicles", hanging structures need a support
 with a minimum diameter. Icicles are vertices that are lower than all
 vertices of all connected triangles.

 Icicles and angled triangles throw a volumetric shadow down. They
 go all the way down to z=0, unless we find a system to only project
 them onto geometry below instead.

 Support can be rendered onto a finished slice, but it must not disturb
 anything that was already rendered, and it must not be rendered above
 the current z height (*1). Other than that, it is a simple projection
 along the z axis.

 *1) by slightly modifying the z height, we generate a layer between
 the support and the model that is less compressed and less sticky.
 This may help a lot with support removal.
 *2) support should not touch the model sideways. This can be acheived
 by redering on bigger circumference and subtracting it befor3
 tracing and filling.
 */

typedef Fl_Menu_Item Fl_Menu_Item_List[];

IAPrinterFDM::IAPrinterFDM(const char *name)
:   super(name)
{
	static Fl_Menu_Item colorMenu[] = {
		{ "monochrome", 0, nullptr, (void*)0, 0, 0, 0, 11 },
		{ "dual color", 0, nullptr, (void*)1, 0, 0, 0, 11 },
		{ nullptr } };

	/** \bug pSettingList must free all members */
	pSettingList.push_back(
		new IASettingChoice("Color:", pColorMode, [this]{userChangedColorMode(); }, colorMenu));
}


/**
 * Save the current slice data to a prepared filename.
 *
 * Verify a given filename when this is the first call in a session. Request
 * a new filename if none was set yet.
 */
void IAPrinterFDM::userSliceSave()
{
    if (pFirstWrite) {
        userSliceSaveAs();
    } else {
        // FIXME: if not yet sliced, so it
        //sliceAll();
        // FIXME: save to disk
        saveToolpath();
    }
}


/**
 * Implement this to open a file chooser with the require file
 * pattern and extension.
 */
void IAPrinterFDM::userSliceSaveAs()
{
    if (queryOutputFilename("Save toolpath as GCode", "*.gcode", ".gcode")) {
        pFirstWrite = false;
        userSliceSave();
    }
}


/**
 * Generate all slice data and cache it for a fast preview or save operation.
 */
void IAPrinterFDM::userSliceGenerateAll()
{
    purgeSlicesAndCaches();
    sliceAll();
}


/**
 * Slice all meshes and models in the scenen.
 */
void IAPrinterFDM::sliceAll()
{
    pMachineToolpath.clear();

    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z() + 2.0*layerHeight();
    // initial height determines stickiness to bed

    double zMin = layerHeight() * 0.9;
    double zLayerHeight = layerHeight();
#if 1
    double zMax = hgt;
#else
    zLayerHeight = 2.0;
    double zMax = hgt + zLayerHeight;
#endif

    IAProgressDialog::show("Genrating slices",
                           "Building shell for layer %d of %d (%d%%)");

    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);
    // create a slice for each layer
    IASlice **sliceList = (IASlice**)calloc(n+2, sizeof(IASlice*));

    for (double z=zMin; z<zMax+2*zLayerHeight; z+=zLayerHeight)
    {
        if (IAProgressDialog::update(i*50/n, i, n, i*50/n)) break;

        IASlice *slc = sliceList[i] = new IASlice( this );
        slc->setNewZ(z);
        slc->generateRim(Iota.pMesh);
        slc->tesselateLidFromRim();
        slc->drawFlat(false, 1, 1, 1);

        auto tp0 = slc->pFramebuffer->toolpathFromLassoAndContract(z, 0.5 * pNozzleDiameter);
        auto tp1 = tp0 ? slc->pFramebuffer->toolpathFromLassoAndContract(z, pNozzleDiameter) : nullptr;
        auto tp2 = tp1 ? slc->pFramebuffer->toolpathFromLassoAndContract(z, pNozzleDiameter) : nullptr;
        auto tp3 = tp2 ? slc->pFramebuffer->toolpathFromLassoAndContract(z, pNozzleDiameter) : nullptr;

        IAToolpathList *tp = pMachineToolpath.createLayer(z);
        if (tp3) tp->add(tp3.get(), 1, 0);
        if (tp2) tp->add(tp2.get(), 1, 1);
        if (tp1) tp->add(tp1.get(), 1, 2);

        i++;
    }

    IAProgressDialog::setText("Building lids and infill for layer %d of %d (%d%%)");
    i=0;
    for (double z=zMin; z<zMax-zLayerHeight; z+=zLayerHeight)
    {
        if (IAProgressDialog::update(i*50/n+50, i, n, i*50/n+50)) break;

        IAToolpathList *tp = pMachineToolpath.findLayer(z);
        IASlice *slc = sliceList[i];

#if 0
        IAFramebuffer lid_mask(sliceList[i+1]->pFramebuffer);
        if (sliceList[i+2] && sliceList[i+2]->pFramebuffer)
            lid_mask.logicAnd(sliceList[i+2]->pFramebuffer);
        IAFramebuffer mask(lid_mask);
        if (i>0) {
            IAFramebuffer bot_mask(sliceList[i-1]->pFramebuffer);
            bot_mask.logicAnd((i>1) ? sliceList[i-2]->pFramebuffer : nullptr);
            mask.logicAnd(&bot_mask);
        }
#else
        IAFramebuffer mask(sliceList[i+1]->pFramebuffer);
        if (sliceList[i+2] && sliceList[i+2]->pFramebuffer)
            mask.logicAnd(sliceList[i+2]->pFramebuffer);
        mask.logicAnd((i>0) ? sliceList[i-1]->pFramebuffer : nullptr);
        mask.logicAnd((i>1) ? sliceList[i-2]->pFramebuffer : nullptr);
#endif

        // build lids and bottoms
        IAFramebuffer lid(slc->pFramebuffer);
        lid.logicAndNot(&mask);
        IAFramebuffer infill(slc->pFramebuffer);
#if 0
        //   build the lid with horizontal and vertical close lines
        // ZIGZAG (could do bridging if used in the correct direction!)
        lid.overlayLidPattern(i, pNozzleDiameter);
        auto lidPath = lid.toolpathFromLasso(z);
        if (lidPath) tp->add(lidPath.get());
#else
        //   build the lid with concentric outlines
        // CONCENTRIC (nicer for lids)
        /** \bug limit this to the widtha and hight of the build platform divided by the extrsuion width */
        int k;
        for (k=0;k<500;k++) {
            auto tp1 = lid.toolpathFromLassoAndContract(z, pNozzleDiameter);
            if (!tp1) break;
            infill.subtract(tp1, pNozzleDiameter);
            tp->add(tp1.get(), 2, k);
        }
        if (k==500) {
            assert(0);
        }
#endif

        // build infills
        /** \todo We are actually filling the areas where the lids and the infill touch twice! */
        /** \todo remove material that we generated in the lid already */
        infill.logicAnd(&mask);

        infill.overlayInfillPattern(i, 3.0);
        auto infillPath = infill.toolpathFromLasso(z);
        if (infillPath) tp->add(infillPath.get(), 3, 0);

        i++;
    }

    i=0;
    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        IASlice *slc = sliceList[i];
        delete slc;
        i++;
    }
    free((void*)sliceList);

    IAProgressDialog::hide();
    if (zRangeSlider->lowValue()>n-1) {
        int nn = n-2; if (nn<0) nn = 0;
        double d = zRangeSlider->highValue()-zRangeSlider->lowValue();
        zRangeSlider->lowValue(nn);
        zRangeSlider->highValue(nn+d);
    }
    zRangeSlider->do_callback();
    gSceneView->redraw();
}


void IAPrinterFDM::saveToolpath(const char *filename)
{
    if (!filename)
        filename = outputPath();
    pMachineToolpath.optimize();
    // generate Toolpath if it is not complete
    pMachineToolpath.saveGCode(filename);
}


/**
 * Create the Treeview items for setting up the printout for this session.
 *
 * \todo number of extrusion in the shell
 * \todo number of layers for lids and bottoms
 * \todo density for infills
 */
void IAPrinterFDM::buildSessionSettings()
{
    wSessionSettings->begin();
    
    char buf[80];

    static Fl_Menu_Item lHgtMenu[] = {
        { "0.1", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.2", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { "0.3", 0, nullptr, nullptr, 0, 0, 0, 11 },
        { }
    };

    /** \bug this keeps on adding Choice widgets to the tree class!
     * We must either delete the widgets when changing printer (which is hard,
     * because the scrollbars are also children of Fl_Tree), or we must
     * store the link to this widget and show and hide it accordingly (which
     * is also not obvious)
     * Or, we override Fl_Tree and write our owb clean() method?
     */
    Fl_Input_Choice *lHgt = new Fl_Input_Choice(1, 1, 60, 1, "mm");
    lHgt->align(FL_ALIGN_RIGHT);
    lHgt->labelsize(11);
    lHgt->textsize(11);
    lHgt->menu(lHgtMenu);
    sprintf(buf, "%.2f", layerHeight()); lHgt->value(buf);
    lHgt->callback(userSetLayerHeightCB, this);
    Fl_Tree_Item *it = wSessionSettings->add("Layer Height: ");
    it->widget(lHgt);

    for (auto &s: pSettingList) {
        s->build();
    }
}


/**
 * The layer height was changed via the layer height chooser in the seesion
 * setting view.
 */
void IAPrinterFDM::userSetLayerHeight(Fl_Input_Choice *w)
{
    /** \todo warn if layer height is 0, negative, or huge */
    pLayerHeight = atof(w->value());
}


/**
 * The color mode was changed via the color mode chooser in the session
 * settings view.
 */
void IAPrinterFDM::userSetColorMode(Fl_Choice *w)
{
    Fl_Menu_Item const* mi = w->mvalue();
    if (mi) {
        pColorMode = (int)(fl_intptr_t)(mi->user_data());
    }
}

void IAPrinterFDM::userChangedColorMode()
{
    printf("Colormode is now %d\n", pColorMode);
}





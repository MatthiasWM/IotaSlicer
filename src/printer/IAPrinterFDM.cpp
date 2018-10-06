//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterFDM.h"

#include "Iota.h"
#include "view/IAGUIMain.h"
#include "view/IAProgressDialog.h"
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



IAPrinterFDM::IAPrinterFDM()
:   super()
{
}


IAPrinterFDM::IAPrinterFDM(IAPrinterFDM const& src)
:   super(src)
{
    pNozzleDiameter = src.pNozzleDiameter;
    pColorMode = src.pColorMode;
    pNumShells = src.pNumShells;
    pNumLids = src.pNumLids;
    pLidType = src.pLidType;
    pInfillDensity = src.pInfillDensity;
    pHasSkirt = src.pHasSkirt;
}


IAPrinterFDM::~IAPrinterFDM()
{
    // nothing to delete
}


IAPrinter *IAPrinterFDM::clone() const
{
    IAPrinterFDM *p = new IAPrinterFDM(*this);
    return p;
}


void IAPrinterFDM::initializePrinterProperties()
{
    super::initializePrinterProperties();
}


void IAPrinterFDM::initializeSceneSettings()
{
    static Fl_Menu_Item numShellsMenu[] = {
        { "0*", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "3", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };

    static Fl_Menu_Item numLidsMenu[] = {
        { "0*", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { nullptr } };

    static Fl_Menu_Item lidTypeMenu[] = {
        { "zigzag", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "concentric", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };

    static Fl_Menu_Item infillDensityMenuMenu[] = {
        { "0%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "5%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "10%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "20%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "30%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "50%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "100%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };

    static Fl_Menu_Item skirtMenu[] = {
        { "no", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "yes", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };

    static Fl_Menu_Item nozzleDiameterMenu[] = {
        { "0.40", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.35", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };
    
    super::initializeSceneSettings();
    IASetting *s;
    s = new IASettingChoice("NPerimiter", "# of perimeters: ", pNumShells,
                            [this]{userChangedNumShells();}, numShellsMenu );
    pSceneSettings.push_back(s);

    s = new IASettingChoice("NLids", "# of lids: ", pNumLids,
                            [this]{userChangedNumLids();}, numLidsMenu );

    pSceneSettings.push_back(s);

    s = new IASettingChoice("lidType", "lid type: ", pLidType,
                            [this]{userChangedLidType();}, lidTypeMenu );
    pSceneSettings.push_back(s);

    s = new IASettingFloatChoice("infillDensity", "infill density: ", pInfillDensity, "%",
                                 [this]{userChangedInfillDensity();}, infillDensityMenuMenu );
    pSceneSettings.push_back(s);

    s = new IASettingChoice("skirt", "skirt: ", pHasSkirt,
                            [this]{userChangedSkirt();}, skirtMenu );
    pSceneSettings.push_back(s);

    s = new IASettingFloatChoice("nozzleDiameter", "nozzle diameter: ", pNozzleDiameter, "mm",
                                 [this]{userChangedNozzleDiameter();}, nozzleDiameterMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item supportMenu[] = {
        { "no", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "yes", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };
    s = new IASettingChoice("support", "support: ", pSupport,
                            [this]{ ; }, supportMenu ); // FIXME: recache all
    pSceneSettings.push_back(s);

    // TODO: support material

    static Fl_Menu_Item supportAngleMenu[] = {
        { "45.0\xC2\xB0", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "50.0\xC2\xB0", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "55.0\xC2\xB0", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "60.0\xC2\xB0", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IASettingFloatChoice("support/angle", "angle: ", pSupportAngle, "deg",
                                 [this]{ ; }, supportAngleMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item supportDensityMenu[] = {
        { "20.0%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "30.0%", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "40.0%", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "45.0%", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "50.0%", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IASettingFloatChoice("support/density", "density: ", pSupportDensity, "%",
                                 [this]{ ; }, supportDensityMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item topGapMenu[] = {
        { "0 layers", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1 layer",  0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2 layers", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "3 layers", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IASettingFloatChoice("support/topGap", "top gap: ", pSupportTopGap, "layers",
                                 [this]{ ; }, topGapMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item sideGapMenu[] = {
        { "0.0mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.1mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.2mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.3mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.4mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };
    s = new IASettingFloatChoice("support/sideGap", "side gap: ", pSupportSideGap, "mm",
                                 [this]{ ; }, sideGapMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item bottomGapMenu[] = {
        { "0 layers", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1 layer",  0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2 layers", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "3 layers", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IASettingFloatChoice("support/bottomGap", "bottom gap: ", pSupportBottomGap, "layers",
                                 [this]{ ; }, bottomGapMenu );
    pSceneSettings.push_back(s);


    // Extrusion width
    // Extrusion speed

    //    static Fl_Menu_Item colorMenu[] = {
    //        { "monochrome", 0, nullptr, (void*)0, 0, 0, 0, 11 },
    //        { "dual color", 0, nullptr, (void*)1, 0, 0, 0, 11 },
    //        { nullptr } };
    //    pSettingList.push_back(
    //        new IASettingChoice("Color:", pColorMode, [this]{userChangedColorMode(); }, colorMenu));

    //    pSettingList.push_back( new IASettingLabel( "test", "Test") );
    //    pSettingList.push_back( new IASettingLabel( "test/toast", "Whitebread") );
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
    IASlice **sliceList = (IASlice**)calloc(n+4, sizeof(IASlice*));

    for (double z=zMin; z<zMax+2*zLayerHeight; z+=zLayerHeight)
    {
        if (IAProgressDialog::update(i*50/n, i, n, i*50/n)) break;

        IASlice *slc = sliceList[i] = new IASlice( this );
        slc->setNewZ(z);
        slc->generateRim(Iota.pMesh);
        slc->tesselateAndDrawLid();
        IAToolpathList *tp = pMachineToolpath.createLayer(z);

        if (pHasSkirt && z==zMin) {
            IAFramebuffer skirt(this, IAFramebuffer::RGBA);
            skirt.bindForRendering();
            glPushMatrix();
            glTranslated(Iota.pMesh->position().x(), Iota.pMesh->position().y(), Iota.pMesh->position().z());
            Iota.pMesh->draw(IAMesh::kMASK, 1.0, 1.0, 1.0);
            glPopMatrix();
            skirt.unbindFromRendering();
            skirt.saveAsJpeg("/Users/matt/aaa.jpg");
            skirt.toolpathFromLassoAndExpand(z, 3);  // 3mm, should probably be more if the extrusion is 1mm or more
            IAToolpathListSP tpSkirt1 = skirt.toolpathFromLassoAndContract(z, pNozzleDiameter);
            tp->add(tpSkirt1.get(), 5, 0);
            IAToolpathListSP tpSkirt2 = skirt.toolpathFromLassoAndContract(z, pNozzleDiameter);
            tp->add(tpSkirt2.get(), 5, 1);
        }

        IAToolpathListSP tp0 = nullptr, tp1 = nullptr, tp2 = nullptr, tp3 = nullptr;
        if (pNumShells>0) {
            tp0 = slc->pFramebuffer->toolpathFromLassoAndContract(z, 0.5 * pNozzleDiameter);
            tp1 = tp0 ? slc->pFramebuffer->toolpathFromLassoAndContract(z, pNozzleDiameter) : nullptr;
        }
        if (pNumShells>1) {
            tp2 = tp1 ? slc->pFramebuffer->toolpathFromLassoAndContract(z, pNozzleDiameter) : nullptr;
        }
        if (pNumShells>2) {
            tp3 = tp2 ? slc->pFramebuffer->toolpathFromLassoAndContract(z, pNozzleDiameter) : nullptr;
        }
        /** \todo We can create an overlap between the infill and the shell by
         *      reducing the second parameter of toolpathFromLassoAndContract
         *      for the last shell that is created.
         */

        if (tp3) tp->add(tp3.get(), 10, 0);
        if (tp2) tp->add(tp2.get(), 10, 1);
        if (tp1) tp->add(tp1.get(), 10, 2);

        i++;
    }

    IAProgressDialog::setText("Building lids and infill for layer %d of %d (%d%%)");
    i=0;
    for (double z=zMin; z<zMax-zLayerHeight; z+=zLayerHeight)
    {
        if (IAProgressDialog::update(i*50/n+50, i, n, i*50/n+50)) break;

        IAToolpathList *tp = pMachineToolpath.findLayer(z);
        IASlice *slc = sliceList[i];

        // --- support structures
        if (pSupport) {
            IAFramebuffer support(this, IAFramebuffer::RGBAZ);

            support.bindForRendering();

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GREATER);

            glClearDepth(0.0);
            glClear(GL_DEPTH_BUFFER_BIT);

            // Draw only what is above the current z plane, but leave a little
            // space between the model and the support to reduce stickyness
            support.beginClipBelowZ(z + pSupportTopGap*layerHeight());

            // FIXME: don;t do this for every layer!
            // mark all triangles that need support first, then render only those
            // mark all vertices that need support, then render them here
            Iota.pMesh->drawAngledFaces(90.0+pSupportAngle);

            glPushMatrix();
            // Draw the upper part of the model, so that the support will not
            // protrude through it. Leave a little gap so that the support
            // Also, remember if we need support at all.
            // sticks less to the model.
            glTranslated(Iota.pMesh->position().x(),
                         Iota.pMesh->position().y(),
                         Iota.pMesh->position().z()
                         + (pSupportBottomGap+pSupportTopGap)*layerHeight());
            Iota.pMesh->draw(IAMesh::kMASK, 0.0, 0.0, 0.0);
            glPopMatrix();

            support.endClip();

            glDepthFunc(GL_LESS);
            glClearDepth(1.0);

            support.unbindFromRendering();
            // FIXME: change to bitmap mode

            // reduce the size of the mask to leave room for the filament, plus
            // a little gap so that the support tower sides do not stick to
            // the model.
            support.toolpathFromLassoAndContract(z, pNozzleDiameter/2.0 + pSupportSideGap);

            // Fill it.
            support.overlayInfillPattern(0, 2*pNozzleDiameter * (100.0 / pSupportDensity) - pNozzleDiameter);
            auto supportPath = support.toolpathFromLasso(z);
            if (supportPath) tp->add(supportPath.get(), 60, 0);
            // TODO: don't draw anything we will draw otherwise
            // FIXME: find icicles and draw support for those
            // FIXME: find and exclude bridges
        }

        // --- infill, lids, bottoms

        IAFramebuffer infill(slc->pFramebuffer);

        // build lids and bottoms
        if (pNumLids>0) {
            IAFramebuffer mask(sliceList[i+1]->pFramebuffer);
            if (pNumLids>1) {
                if (sliceList[i+2] && sliceList[i+2]->pFramebuffer)
                    mask.logicAnd(sliceList[i+2]->pFramebuffer);
                else
                    mask.clear();
            }
            mask.logicAnd((i>0) ? sliceList[i-1]->pFramebuffer : nullptr);
            if (pNumLids>1) {
                mask.logicAnd((i>1) ? sliceList[i-2]->pFramebuffer : nullptr);
            }

            IAFramebuffer lid(slc->pFramebuffer);
            lid.logicAndNot(&mask);
            infill.logicAnd(&mask);

            if (pLidType==0) {
                // ZIGZAG (could do bridging if used in the correct direction!)
                lid.overlayInfillPattern(i, pNozzleDiameter);
                auto lidPath = lid.toolpathFromLasso(z);
                if (lidPath) tp->add(lidPath.get(), 20, 0);
            } else {
                // CONCENTRIC (nicer for lids)
                /** \bug limit this to the width and hight of the build platform divided by the extrsuion width */
                int k;
                for (k=0;k<300;k++) { // FIXME
                    auto tp1 = lid.toolpathFromLassoAndContract(z, pNozzleDiameter);
                    if (!tp1) break;
                    infill.subtract(tp1, pNozzleDiameter);
                    tp->add(tp1.get(), 20, k);
                }
                if (k==300) {
                    // assert(0);
                }
            }
        }

        // build infills
        /** \todo We are actually filling the areas where the lids and the infill touch twice! */
        /** \todo remove material that we generated in the lid already */

        if (pInfillDensity>0.0001) {
            // pNozzleDiameter = 100%
            // pNozzleDiameter*2 = 50%
            // pNozzleDiameter*4 = 25%
            infill.overlayInfillPattern(i, 2*pNozzleDiameter * (100.0 / pInfillDensity) - pNozzleDiameter);
            auto infillPath = infill.toolpathFromLasso(z);
            if (infillPath) tp->add(infillPath.get(), 30, 0);
        }

        i++;
    }

    for (i=0; i<n+4; i++) {
        IASlice *slc = sliceList[i];
        delete slc;
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
        filename = recentUploadFilename();
    pMachineToolpath.optimize();
    // generate Toolpath if it is not complete
    pMachineToolpath.saveGCode(filename);
}


/**
 * Clear all buffered data and prepare for a modified scene.
 */
void IAPrinterFDM::purgeSlicesAndCaches()
{
    pMachineToolpath.clear();
    super::purgeSlicesAndCaches();
}


/**
 * Draw a preview of the slicing operation.
 */
void IAPrinterFDM::drawPreview(double lo, double hi)
{
    pMachineToolpath.draw(lo, hi);
}




void IAPrinterFDM::userChangedColorMode()
{
    printf("Colormode is now %d\n", pColorMode);
    // TODO: clear toolpath and slice cache
}


void IAPrinterFDM::userChangedNumLids()
{
    // TODO: clear toolpath and slice cache
}


void IAPrinterFDM::userChangedNumShells()
{
    // TODO: clear toolpath and slice cache
}


void IAPrinterFDM::userChangedLidType()
{
    // TODO: clear toolpath and slice cache
}

void IAPrinterFDM::userChangedInfillDensity()
{
    // TODO: clear toolpath and slice cache
}

void IAPrinterFDM::userChangedSkirt()
{
    // TODO: clear toolpath and slice cache
}

void IAPrinterFDM::userChangedNozzleDiameter()
{
    // TODO: clear toolpath and slice cache
}






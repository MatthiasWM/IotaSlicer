//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFDMPrinter.h"

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

/*
 Material Propertis

 Chemistry (ABS, PLA, ...)
 Build temperature


 FDM Printer properties:

 controller
    port type (USB, TCP/IP, OctoPrint)
    port name
    dialect
    GCode: start (call an editor window)
    GCode: end, ...
 buildvolume
    printbed type (rect, round)
    printbed width, depth
    voulme height (can be infinite...)
    origin (front left, back right)
    origin offset (x, y, z)
    heated bed
    heated case
    fan number
    motion
        build speed (x, y, z)
        travel speed (x, y, z)
        acceleration (x, y, z)
        endstop (+ or -) (x, y, z)
        backlash?
        head weight, vibration frequency, ...
 # of hotends
    hotend# (heater number)
        hotend type (mono, color switching, color mixing)
        filament diameter
        nozzle diameter
        fan number
        feed type (direct, bowden)
        # of feeds
            feed# (extruder number)
                default filament
                extruder compensation
                fan?
                filament diameter?
                drive type?
        position of waste station (x, y, x2, y2)
        position of wipe, direction of wipe (x, y, dx, dy)


 FDM Print Settings:

 preset Machine overrides
    hotend#: nozzle diameter
    feed#.#: filament
 preset Construction (fast, good, high, custom...)
    layer height
    number of shells
        speed factor outer shell
    infille density
        infill pattern (plus more settings related to the pattern)
        filament
    number of lids
        lid infill pattern
    nuber of bottoms
        lid infill pattern
        bridge settings
 preset Support:
    density
    pattern
    gaps
    material
 preset Material:
    color type (per mesh, texture, rainbow)
    virtual color (preset for mixing extruder)
    ...
 Mesh List:
    mesh# name
        override
            Quality preset: (default, by name)
            Support preset: (default, by name)
            Material preset: (default, by name)
        color (if preset is "color per mesh")
    ...

 */

typedef Fl_Menu_Item Fl_Menu_Item_List[];



IAFDMPrinter::IAFDMPrinter()
:   super()
{
}


IAFDMPrinter::IAFDMPrinter(IAFDMPrinter const& src)
:   super(src)
{
    numExtruders.set( src.numExtruders() );

    nozzleDiameter = src.nozzleDiameter;
    numShells.set( src.numShells() );
    numLids.set( src.numLids() );
    lidType.set( src.lidType() );
    infillDensity = src.infillDensity;
    hasSkirt.set( src.hasSkirt() );
    minimumLayerTime.set( src.minimumLayerTime() );
    // FIXME: annd all other properties and settings
}


IAFDMPrinter::~IAFDMPrinter()
{
    // nothing to delete
}


IAPrinter *IAFDMPrinter::clone() const
{
    IAFDMPrinter *p = new IAFDMPrinter(*this);
    // FIXME: not working
    return p;
}


//+         printer name: WonkoPrinter
//            connection: Disk/USB/Network/OctoPrint/Stick,SD
//                  path: ...
//                  test: [click]
//+ range of motion from: x, y, z
//+                   to: x, y, z
//           x direction: left to right/right to left
//           y direction: front to back/back to front
//           build plate: rectangular/round
//+  printable area from: x, y
//+                   to: x, y
//            heated bed: (heater number)
//           heated case: (heater number)
//        # of extruders: 2
//              extruder 1:
//                    offset: x, y
//         filament diameter: 1.75
//           nozzle diameter: 0.35
//                      type: single/changing/mixing
//                # of feeds: 4
//      GCode start extruder: script or GCode
//              end extruder: script or GCode
//              extruder 2: ...
//           GCode start: script or GCode
//                   end: script or GCode
//               dialect: Repetier


void IAFDMPrinter::createPropertiesControls()
{
    IATreeItemController *s;

    super::createPropertiesControls();

    s = new IALabelController("specs", "Specifications:");
    pPropertiesControllerList.push_back(s);
    s = new IAVectorController("specs/motionRangeMin", "Range of Motion, minimum:", "", motionRangeMin,
                               "From X:", "mm",
                               "Y:", "mm",
                               "Z:", "mm", []{} );
    pPropertiesControllerList.push_back(s);
    s = new IAVectorController("specs/motionRangeMax", "maximum:", "", motionRangeMax,
                               "To X:", "mm (Width)",
                               "Y:", "mm (Depth)",
                               "Z:", "mm (Height)", []{} );
    pPropertiesControllerList.push_back(s);
    s = new IAVectorController("specs/printVolumeMin", "Printable Area, minimum:", "", printVolumeMin,
                               "From X:", "mm",
                               "Y:", "mm",
                               nullptr, nullptr, []{} );
    pPropertiesControllerList.push_back(s);
    s = new IAVectorController("specs/printVolumeMax", "maximum:", "", printVolumeMax,
                               "To X:", "mm (Width)",
                               "Y:", "mm (Depth)",
                               nullptr, nullptr, []{} );
    pPropertiesControllerList.push_back(s);
    // travel speed, print speed, build plate fan, build plate heater, chamber heater
    static Fl_Menu_Item numExtruderMenu[] = {
        { "1", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAChoiceController("specs/extruder", "Extruders:", numExtruders,
                               []{}, numExtruderMenu );
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0", "Extruder 0:");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/offset", "Offset:");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/offset/x", "X:", "mm");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/offset/y", "Y:", "mm");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/filamentDiameter", "Filament Diameter:", "mm");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/nozzleDiameter", "Nozzle Diameter:", "mm");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/feedrate", "Max. Feedrate:", "mm/min");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/fan", "Fan ID:", "1");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/feedType", "Feed Type:", "direct/bowden");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/type", "Type:", "single/changeing/mixing");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/0/feeds", "# of Feeds:", "4");
    pPropertiesControllerList.push_back(s);
    s = new IALabelController("specs/extruder/1", "Extruder 1:");
    pPropertiesControllerList.push_back(s);
    // :construction: = \xF0\x9F\x9A\xA7
    // :warning: = E2 9A A0
    // :stop: = F0 9F 9B 91
}


void IAFDMPrinter::initializeSceneSettings()
{
    super::initializeSceneSettings();
    IATreeItemController *s;

    // This preset contains all costumizable changes to the printer hardware.
    // Mainly, this is the filament used, and in rare cases a change in nozzle
    // type or diameter.
    // For mixing extruders, this can also include virtual extruders.
    // For now, we may also want to put layer height in here, until we allow
    // different layer heights for different models.
    static Fl_Menu_Item printerPresetMenu[] = {
        // if any of the setting within the preset changed, mark it as "not yet saved"
        // (or as one of the other preset, in case they match)
        { "<not saved>", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        // show the item below, if the preset is marked as "not yet saved"
        { "save this as a preset...", 0, nullptr, (void*)0, FL_MENU_DIVIDER, 0, 0, 11 },
        // show this item, if the preset exists. If it is "default", deactivate this menu
        { "remove this preset...", 0, nullptr, (void*)0, FL_MENU_DIVIDER|FL_MENU_INVISIBLE, 0, 0, 11 },
        // "default" means, that all settings will be the printer default properties
        { "default", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        // a list of custom named and generated preset that will be saved to a file
        // and that can be selectd on a pre-mesh basis in the settings below
        { "soluble support", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "calibrated color", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { nullptr } };
    static IAIntProperty printerPreset { "printerPreset", 0 };
    s = new IAChoiceController("Printer", "Printer:", printerPreset,
                               []{;}, printerPresetMenu);
    pSceneSettings.push_back(s);
    // extruder 0 nozzle size
    // extruder 0 material and color
    // extruder 1 nozzle size
    // extruder 1 material and color


    // The next list of parameters are presets for print quality. They determine
    // home meshes are converted into plastic strings.
    static Fl_Menu_Item numShellsMenu[] = {
        { "0*", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "3", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };

    s = new IAChoiceController("NPerimiter", "# of perimeters: ", numShells,
                         []{;}, numShellsMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item numLidsMenu[] = {
        { "0*", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { nullptr } };

    s = new IAChoiceController("NLids", "# of lids: ", numLids,
                         [this]{userChangedNumLids();}, numLidsMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item lidTypeMenu[] = {
        { "zigzag", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "concentric", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };

    s = new IAChoiceController("lidType", "lid type: ", lidType,
                            [this]{userChangedLidType();}, lidTypeMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item infillDensityMenuMenu[] = {
        { "0%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "5%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "10%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "20%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "30%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "50%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "100%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };

    s = new IAFloatChoiceController("infillDensity", "infill density: ", infillDensity, "%",
                                 [this]{userChangedInfillDensity();}, infillDensityMenuMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item skirtMenu[] = {
        { "no", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "yes", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };

    s = new IAChoiceController("skirt", "skirt: ", hasSkirt,
                            [this]{userChangedSkirt();}, skirtMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item layerTimeMenu[] = {
        { "0 sec.", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "15 sec.", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "30 sec.", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAFloatChoiceController("minLayerTime", "min. layer time: ", minimumLayerTime, "sec",
                                    []{;}, layerTimeMenu );
    s->tooltip("This ia the minimum time it will take to print a layer. Setting this "
               "to 15 seconds or more will give already printed filament time to cool "
               "before the next layer is added.");
    pSceneSettings.push_back(s);

    static Fl_Menu_Item extruderChoiceMenu[] = {
        { "#0 (white)", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "#1 (black)", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAChoiceController("modelExtruder", "extruder:", modelExtruder,
                               []{}, extruderChoiceMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item nozzleDiameterMenu[] = {
        { "0.40", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.35", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };

    s = new IAFloatChoiceController("nozzleDiameter", "nozzle diameter: ", nozzleDiameter, "mm",
                                 [this]{userChangedNozzleDiameter();}, nozzleDiameterMenu );
    pSceneSettings.push_back(s);

    // There will be a preset system for the way that support structures are
    // built. Not every mesh in an assembly needs support.
    static Fl_Menu_Item supportMenu[] = {
        { "no", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "yes", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAChoiceController("support", "support: ", hasSupport,
                         []{ ; }, supportMenu ); // FIXME: recache all
    pSceneSettings.push_back(s);

    // TODO: support material

    static Fl_Menu_Item supportAngleMenu[] = {
        { "45.0\xC2\xB0", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "50.0\xC2\xB0", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "55.0\xC2\xB0", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "60.0\xC2\xB0", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAFloatChoiceController("support/angle", "angle: ", supportAngle, "deg",
                                 []{ ; }, supportAngleMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item supportDensityMenu[] = {
        { "20.0%", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "30.0%", 0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "40.0%", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "45.0%", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "50.0%", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAFloatChoiceController("support/density", "density: ", supportDensity, "%",
                                 []{ ; }, supportDensityMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item topGapMenu[] = {
        { "0 layers", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1 layer",  0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2 layers", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "3 layers", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAFloatChoiceController("support/topGap", "top gap: ", supportTopGap, "layers",
                                 []{ ; }, topGapMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item sideGapMenu[] = {
        { "0.0mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.1mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.2mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.3mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "0.4mm", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAFloatChoiceController("support/sideGap", "side gap: ", supportSideGap, "mm",
                                 []{ ; }, sideGapMenu );
    pSceneSettings.push_back(s);

    static Fl_Menu_Item bottomGapMenu[] = {
        { "0 layers", 0, nullptr, (void*)0, 0, 0, 0, 11 },
        { "1 layer",  0, nullptr, (void*)1, 0, 0, 0, 11 },
        { "2 layers", 0, nullptr, (void*)2, 0, 0, 0, 11 },
        { "3 layers", 0, nullptr, (void*)3, 0, 0, 0, 11 },
        { nullptr } };
    s = new IAFloatChoiceController("support/bottomGap", "bottom gap: ", supportBottomGap, "layers",
                                 []{ ; }, bottomGapMenu );
    pSceneSettings.push_back(s);

    // We need an extruder ref controller that displays the current material and
    // color for the choosen extruder. For mixing extruders, this could even allow
    // a choice of color?
    s = new IAChoiceController("support/extruder", "extruder:", supportExtruder,
                               []{}, extruderChoiceMenu );
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
void IAFDMPrinter::userSliceSave()
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
void IAFDMPrinter::userSliceSaveAs()
{
    if (queryOutputFilename("Save toolpath as GCode", "*.gcode", ".gcode")) {
        pFirstWrite = false;
        userSliceSave();
    }
}


/**
 * Generate all slice data and cache it for a fast preview or save operation.
 */
void IAFDMPrinter::userSliceGenerateAll()
{
    purgeSlicesAndCaches();
    sliceAll();
}


/**
 * Create and add the toolpath for a skirt around the omesh base.
 */
void IAFDMPrinter::addToolpathForSkirt(IAToolpathList *tp, double z)
{
    IAFramebuffer skirt(this, IAFramebuffer::RGBA);
    skirt.bindForRendering();
    glPushMatrix();
    glTranslated(Iota.pMesh->position().x(), Iota.pMesh->position().y(), Iota.pMesh->position().z());
    Iota.pMesh->draw(IAMesh::kMASK, 1.0, 1.0, 1.0);
    glPopMatrix();
    skirt.unbindFromRendering();
    skirt.toolpathFromLassoAndExpand(z, 3);  // 3mm, should probably be more if the extrusion is 1mm or more
    IAToolpathListSP tpSkirt1 = skirt.toolpathFromLassoAndContract(z, nozzleDiameter());
    tp->add(tpSkirt1.get(), modelExtruder(), 5, 0);
    IAToolpathListSP tpSkirt2 = skirt.toolpathFromLassoAndContract(z, nozzleDiameter());
    tp->add(tpSkirt2.get(), modelExtruder(), 5, 1);
}


/**
 * Create and add the toolpath for a support structure under overhangs.
 */
void IAFDMPrinter::addToolpathForSupport(IAToolpathList *tp, double z, int i)
{
    IAFramebuffer support(this, IAFramebuffer::RGBAZ);

    support.bindForRendering();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glClearDepth(0.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw only what is above the current z plane, but leave a little
    // space between the model and the support to reduce stickyness
    support.beginClipBelowZ(z + supportTopGap()*layerHeight());

    // FIXME: don't do this for every layer!
    // mark all triangles that need support first, then render only those
    // mark all vertices that need support, then render them here
    Iota.pMesh->drawAngledFaces(90.0+supportAngle());

    glPushMatrix();
    // Draw the upper part of the model, so that the support will not
    // protrude through it. Leave a little gap so that the support
    // Also, remember if we need support at all.
    // sticks less to the model.
    glTranslated(Iota.pMesh->position().x(),
                 Iota.pMesh->position().y(),
                 Iota.pMesh->position().z()
                 + (supportBottomGap()+supportTopGap())*layerHeight());
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
    support.toolpathFromLassoAndContract(z, nozzleDiameter()/2.0 + supportSideGap());

    // Fill it.
    if (i==0) {
        // layer 0 must be filled 100% to give good adhesion
        support.overlayInfillPattern(0, nozzleDiameter());
    } else {
        // other layers use the set density
        support.overlayInfillPattern(0, 2*nozzleDiameter() * (100.0 / supportDensity()) - nozzleDiameter());
    }
    auto supportPath = support.toolpathFromLasso(z);
    if (supportPath) tp->add(supportPath.get(), supportExtruder(), 60, 0);
    // TODO: don't draw anything here which we will draw otherwise later
    // FIXME: find icicles and draw support for those
    // FIXME: find and exclude bridges
}


/**
 * Create the toolpath to add a shell around the model.
 *
 * when we enter, the framebuffer contains a slice of the entire model
 * on exit, the framebuffer is the core, i.e. the slice minus the outlines
 */
void IAFDMPrinter::addToolpathForShell(IAToolpathList *tp, double z, IAFramebuffer *fb)
{
    IAToolpathListSP tp0 = nullptr, tp1 = nullptr, tp2 = nullptr, tp3 = nullptr;
    if (numShells()>0) {
        tp0 = fb->toolpathFromLassoAndContract(z, 0.5 * nozzleDiameter());
        tp1 = tp0 ? fb->toolpathFromLassoAndContract(z, nozzleDiameter()) : nullptr;
    }
    if (numShells()>1) {
        tp2 = tp1 ? fb->toolpathFromLassoAndContract(z, nozzleDiameter()) : nullptr;
    }
    if (numShells()>2) {
        tp3 = tp2 ? fb->toolpathFromLassoAndContract(z, nozzleDiameter()) : nullptr;
    }
    /** \todo We can create an overlap between the infill and the shell by
     *      reducing the second parameter of toolpathFromLassoAndContract
     *      for the last shell that is created.
     */

    if (tp3) tp->add(tp3.get(), modelExtruder(), 10, 0);
    if (tp2) tp->add(tp2.get(), modelExtruder(), 10, 1);
    if (tp1) tp->add(tp1.get(), modelExtruder(), 10, 2);
}


void IAFDMPrinter::addToolpathForLid(IAToolpathList *tp, double z, int i, IAFramebuffer &lid)
{
    if (lidType()==0) {
        // ZIGZAG (could do bridging if used in the correct direction!)
        lid.overlayInfillPattern(i, nozzleDiameter());
        auto lidPath = lid.toolpathFromLasso(z);
        if (lidPath) tp->add(lidPath.get(), modelExtruder(), 20, 0);
    } else {
        // CONCENTRIC (nicer for lids)
        /** \bug limit this to the width and hight of the build platform divided by the extrsuion width */
        int k;
        for (k=0;k<300;k++) { // FIXME
            auto tp1 = lid.toolpathFromLassoAndContract(z, nozzleDiameter());
            if (!tp1) break;
            tp->add(tp1.get(), modelExtruder(), 20, k);
        }
        if (k==300) {
            // assert(0);
        }
    }
}


void IAFDMPrinter::addToolpathForInfill(IAToolpathList *tp, double z, int i, IAFramebuffer &infill)
{
    /** \todo We are actually filling the areas twice, where the lids and the infill touch! */
    /** \todo remove material that we generated in the lid already */
    infill.overlayInfillPattern(i, 2*nozzleDiameter() * (100.0 / infillDensity()) - nozzleDiameter());
    auto infillPath = infill.toolpathFromLasso(z);
    if (infillPath) tp->add(infillPath.get(), modelExtruder(), 30, 0); // FIXME: should me ExtruderDontCare
}


/**
 * Slice all meshes and models in the scene.
 */
void IAFDMPrinter::sliceAll()
{
    pMachineToolpath.purge();
    pSliceMap.clear();

    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z() + 2.0*layerHeight();
    double zMin = layerHeight() * 0.9; // initial height
    double zLayerHeight = layerHeight();
    double zMax = hgt;

    IAProgressDialog::show("Generating slices",
                           "Building shell for layer %d of %d (%d%%)");

    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);

    for (double z=zMin; z<zMax+2*zLayerHeight; z+=zLayerHeight)
    {
        if (IAProgressDialog::update(i*50/n, i, n, i*50/n)) break;

        IAToolpathList *tp = pMachineToolpath.createLayer(z);

        if (hasSkirt() && z==zMin)
            addToolpathForSkirt(tp, z);

        IAMeshSlice *slc = pSliceMap[i].pMeshSlice = new IAMeshSlice( this );
        slc->setNewZ(z);
        slc->generateRim(Iota.pMesh);
        slc->tesselateAndDrawLid();

        addToolpathForShell(tp, z, slc->pFramebuffer);

        i++;
    }

    IAProgressDialog::setText("Building lids and infill for layer %d of %d (%d%%)");
    i=0;
    for (double z=zMin; z<zMax-zLayerHeight; z+=zLayerHeight)
    {
        if (IAProgressDialog::update(i*50/n+50, i, n, i*50/n+50)) break;

        IAToolpathList *tp = pMachineToolpath.findLayer(z);
        IAMeshSlice *slc = pSliceMap[i].pMeshSlice;

        // support structures
        if (hasSupport())
            addToolpathForSupport(tp, z, i);

        IAFramebuffer infill(slc->pFramebuffer);

        // build lids and bottoms
        if (numLids()>0) {
            IAFramebuffer mask(pSliceMap[i+1].pMeshSlice->pFramebuffer);
            if (numLids()>1) {
                if (pSliceMap[i+2].pMeshSlice && pSliceMap[i+2].pMeshSlice->pFramebuffer)
                    mask.logicAnd(pSliceMap[i+2].pMeshSlice->pFramebuffer);
                else
                    mask.clear();
            }
            mask.logicAnd((i>0) ? pSliceMap[i-1].pMeshSlice->pFramebuffer : nullptr);
            if (numLids()>1) {
                mask.logicAnd((i>1) ? pSliceMap[i].pMeshSlice->pFramebuffer : nullptr);
            }

            IAFramebuffer lid(slc->pFramebuffer);
            lid.logicAndNot(&mask); // TODO: shrink lid
            infill.logicAnd(&mask); // TODO: shrink infill
            addToolpathForLid(tp, z, i, lid);
        }

        // build infills
        if (infillDensity()>0.0001)
            addToolpathForInfill(tp, z, i, infill);

        i++;
    }

    pSliceMap.clear();

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


void IAFDMPrinter::saveToolpath(const char *filename)
{
    if (!filename)
        filename = recentUpload();
    pMachineToolpath.optimize();
    // generate Toolpath if it is not complete
    pMachineToolpath.saveGCode(filename);
}


/**
 * Clear all buffered data and prepare for a modified scene.
 */
void IAFDMPrinter::purgeSlicesAndCaches()
{
    pMachineToolpath.purge();
    super::purgeSlicesAndCaches();
}


/**
 * Draw a preview of the slicing operation.
 */
void IAFDMPrinter::drawPreview(double lo, double hi)
{
    pMachineToolpath.draw(lo, hi);
}




void IAFDMPrinter::userChangedColorMode()
{
    // TODO: clear toolpath and slice cache
}


void IAFDMPrinter::userChangedNumLids()
{
    // TODO: clear toolpath and slice cache
}


void IAFDMPrinter::userChangedNumShells()
{
    // TODO: clear toolpath and slice cache
}


void IAFDMPrinter::userChangedLidType()
{
    // TODO: clear toolpath and slice cache
}

void IAFDMPrinter::userChangedInfillDensity()
{
    // TODO: clear toolpath and slice cache
}

void IAFDMPrinter::userChangedSkirt()
{
    // TODO: clear toolpath and slice cache
}

void IAFDMPrinter::userChangedNozzleDiameter()
{
    // TODO: clear toolpath and slice cache
}


void IAFDMPrinter::readProperties(Fl_Preferences &printer)
{
    super::readProperties(printer);
    Fl_Preferences properties(printer, "properties");
    numExtruders.read(properties);
}


void IAFDMPrinter::writeProperties(Fl_Preferences &printer)
{
    super::writeProperties(printer);
    Fl_Preferences properties(printer, "properties");
    numExtruders.write(properties);
}





IAFDMSlice::IAFDMSlice()
{
}


IAFDMSlice::~IAFDMSlice()
{
    purge();
}


void IAFDMSlice::purge()
{
    delete pMeshSlice; pMeshSlice = nullptr;
    delete pCore; pCore = nullptr;
}

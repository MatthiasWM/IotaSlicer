//
//  IAPrinterFDM.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_FDM_H
#define IA_PRINTER_FDM_H


#include "printer/IAPrinter.h"


class IAFDMPrinter;
class IAFDMSlice;
typedef std::map<double, IAFDMSlice> IAFDMSliceMap;


/**
 * This class holds a single slice of infomeation for a given Z valiue.
 */
class IAFDMSlice
{
public:
    IAFDMSlice(IAFDMPrinter *printer);

private:
    IAFDMPrinter *pPrinter = nullptr;
};


/**
 * FDM, Fused Deposition Modeling, describes printers that extrude filament
 * to create 3D models.
 *
 * This printer driver writes GCode files.
 */
class IAFDMPrinter : public IAPrinter
{
    typedef IAPrinter super;
public:
    // ---- constructor and destructor
    IAFDMPrinter();
    IAFDMPrinter(IAFDMPrinter const& src);
    virtual ~IAFDMPrinter() override;
    IAFDMPrinter &operator=(IAFDMPrinter&) = delete;
    virtual IAPrinter *clone() const override;
    virtual const char *type() const override { return "IAPrinterFDM"; }

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

    virtual void purgeSlicesAndCaches() override;
    virtual void drawPreview(double lo, double hi) override;

    // ---- controllers
    virtual void createPropertiesControls() override;

    // ---- properties
    virtual void readProperties(Fl_Preferences &p) override;
    virtual void writeProperties(Fl_Preferences &p) override;

    IAIntProperty numExtruders { "numExtruders", 2 };
    // ex 0 type
    // ex 0 nozzle diameter
    // ex 0 feeds
    // ex 0 feed 0 material

    // ---- scene settings
    virtual void initializeSceneSettings() override;


    // printer
    IAFloatProperty nozzleDiameter { "nozzleDiameter", 0.4 };
    // construction
    IAIntProperty numShells { "numShells", 3 };
    IAIntProperty numLids { "numLids", 2 };
    IAIntProperty lidType { "lidType", 0 }; // 0=zigzag, 1=concentric
    IAFloatProperty infillDensity { "infillDensity", 20.0 }; // %
    IAIntProperty hasSkirt { "hasSkirt",  1 }; // prime line around perimeter
    IAFloatProperty minimumLayerTime { "minimumLayerTime", 15.0 };
    IAExtruderProperty modelExtruder { "modelExtruder", 0 };
    // support
    IAIntProperty hasSupport { "hasSupport", 1 };
    IAFloatProperty supportAngle { "supportAngle", 50.0 };
    IAFloatProperty supportDensity { "supportDensity", 40.0 };
    IAFloatProperty supportTopGap { "supportTopGap", 1.0 };
    IAFloatProperty supportSideGap { "supportSideGap", 0.2 };
    IAFloatProperty supportBottomGap { "supportBottomGap", 1.0 };
    IAExtruderProperty supportExtruder { "supportExtruder", 0 };
    // models and meshes

    // ----
    void sliceAll();
    void saveToolpath(const char *filename = nullptr);

    double filamentDiameter() { return 1.75; }

protected:
    void userChangedColorMode();
    void userChangedNumLids();
    void userChangedLidType();
    void userChangedNumShells();
    void userChangedInfillDensity();
    void userChangedSkirt();
    void userChangedNozzleDiameter();

private:

    /// the toolpath for the entire scene for vector based machines
    IAMachineToolpath pMachineToolpath = IAMachineToolpath(this);
    IAFDMSliceMap pSliceMap;
};


#endif /* IA_PRINTER_FDM_H */



//
//  IAPrinterFDM.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_FDM_H
#define IA_PRINTER_FDM_H


#include "printer/IAPrinter.h"

#include <mutex>


class IAFDMPrinter;
class IAFDMSlice;


class IAFDMSliceList
{
public:
    IAFDMSliceList() { }
    ~IAFDMSliceList() { }
    /** \todo implement asynchrnous calculation of slices. */
    // if ( m.find("f") == m.end() )
    // lock this list and individual slices
    IAFDMSlice &operator[](int i) { return pList[i]; }
    void purge();
private:
    std::map<int, IAFDMSlice> pList;
};


/**
 * This class holds a single slice of information for a given Z value.
 * \bug per mesh? per scene?
 */
class IAFDMSlice
{
public:
    IAFDMSlice();
    ~IAFDMSlice();
    void purge();
    void lock() { pMutex.lock(); }
    void unlock() { pMutex.unlock(); }

    std::mutex pMutex;
//private:
    /// Store the toolptah that generates the shell
    IAToolpathList *pShellToolpath = nullptr;
    IAToolpathList *pLidToolpath = nullptr;
    IAToolpathList *pInfillToolpath = nullptr;
    IAToolpathList *pSkirtToolpath = nullptr;
    IAToolpathList *pSupportToolpath = nullptr;
    /// Store the bitmap for the slice without the shell
    IAFramebuffer *pCoreBitmap = nullptr;
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
    virtual void rangeSliderChanged() override;

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
    IAPresetProperty supportPreset { presetClass, "supportPreset", "none" };
    IAIntProperty hasSupport { "hasSupport", 0 };
    IAFloatProperty supportAngle { "supportAngle", 50.0 };
    IAFloatProperty supportDensity { "supportDensity", 40.0 };
    IAFloatProperty supportTopGap { "supportTopGap", 1.0 };
    IAFloatProperty supportSideGap { "supportSideGap", 0.2 };
    IAFloatProperty supportBottomGap { "supportBottomGap", 1.0 };
    IAExtruderProperty supportExtruder { "supportExtruder", 0 };
    // models and meshes
    
    // ----
    double sliceIndexToZ(int i);

    void acquireCorePattern(int i);

    void sliceLayer(int i);
    void sliceAll();

    void addToolpathForSkirt(IAToolpathList *tp, int i);
    void addToolpathForSupport(IAToolpathList *tp, int i);
    void createToolpathForShell(int i, IAFramebuffer *slice);
    void addToolpathForLid(IAToolpathList *tp, int i, IAFramebuffer &fb);
    void addToolpathForInfill(IAToolpathList *tp, int i, IAFramebuffer &fb);

    void saveToolpath(const char *filename = nullptr);

    double filamentDiameter() { return 1.75; }
    
private:

    IAFDMSliceList pSliceList;
};


#endif /* IA_PRINTER_FDM_H */



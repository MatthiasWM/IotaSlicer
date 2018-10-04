//
//  IAPrinterLaser.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_LASERCUTTER_H
#define IA_PRINTER_LASERCUTTER_H

#include "printer/IAPrinter.h"

class Fl_Widget;
class Fl_Choice;
class Fl_Input_Choice;


/**
 * The Lasercutter driver generates slices that can be cut and clued together.
 *
 * This printer driver generates DXF files for laser engravers.
 *
 * No color or multimaterial support.
 */
class IAPrinterLasercutter : public IAPrinter
{
    typedef IAPrinter super;
public:
    IAPrinterLasercutter();
    IAPrinterLasercutter(IAPrinterLasercutter const& src);
    virtual ~IAPrinterLasercutter() override;
    IAPrinterLasercutter &operator=(IAPrinterLasercutter&) = delete;
    virtual IAPrinter *clone() const override;
    virtual const char *type() const override { return "IAPrinterLasercutter"; }

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

    // ----
    void sliceAll();
    void saveToolpaths(const char *filename = nullptr);
};


#endif /* IA_PRINTER_LASERCUTTER_H */



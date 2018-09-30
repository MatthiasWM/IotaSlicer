//
//  IAPrinterInkjet.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_INKJET_H
#define IA_PRINTER_INKJET_H

class Fl_Widget;
class Fl_Choice;
class Fl_Input_Choice;

#include "printer/IAPrinter.h"


/**
 * Inkjet base printers include binder-on-powder and multijet printer.
 *
 * This printer driver produces images of slices, containing usually
 * additional components besides RGB. The alpha channel can be used to
 * determine where binder needs to be sprayed. Some multijet printer
 * have additional heads for support structures.
 *
 * Output format can be a list of image files, or a machine dependent
 * code that controlls ink heads and powder spreading alike.
 */
class IAPrinterInkjet : public IAPrinter
{
    typedef IAPrinter super;
public:
    IAPrinterInkjet(const char *name) : super(name) { }
    virtual IAPrinter *clone() override;
    virtual const char *type() override { return "IAPrinterInkjet"; }

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

    // ----
    void sliceAll();
    void saveSlices(const char *filename = nullptr);
    
};


#endif /* IA_PRINTER_INKJET_H */



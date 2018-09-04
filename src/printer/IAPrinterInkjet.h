//
//  IAPrinterInkjet.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_INKJET_H
#define IA_PRINTER_INKJET_H


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
public:
    IAPrinterInkjet(const char *name) : IAPrinter(name) { }
};


#endif /* IA_PRINTER_INKJET_H */



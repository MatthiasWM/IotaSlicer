//
//  IAPrinter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_H
#define IA_PRINTER_H


#include "geometry/IAVector3d.h"

#include <vector>

class Fl_Menu_Item;


/**
 * Manage different types of 3D printers.
 */
class IAPrinter
{
public:
    IAPrinter();
    IAPrinter(const char *name);
    ~IAPrinter();
    void draw();
    void setName(const char *name);
    const char *name();

    IAVector3d pBuildVolume = { 214.0, 214.0, 230.0 };
//    IAVector3d pBuildVolume = { 214.0, 214.0, 330.0 };
    IAVector3d pBuildVolumeMin = { 0.0, 0.0, 0.0 };
    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 230.0 };
//    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 330.0 };
    double pBuildVolumeRadius = 200.0; // sphere that contains the entire centered build volume

private:
    char *pName = nullptr;
};


/**
 * Manage a list of printers.
 */
class IAPrinterList
{
public:
    IAPrinterList(Fl_Menu_Item *printermenu);
    ~IAPrinterList();
    bool add(IAPrinter *printer, const char *name);
    IAPrinter *defaultPrinter();
    void userSelectedPrinter(IAPrinter *p);

private:
    void buildMenuArray();
    static void printerSelectedCB(Fl_Menu_Item*, void *p);

    Fl_Menu_Item *pMenuItem = nullptr;
    Fl_Menu_Item *pMenuArray = nullptr;

    std::vector<IAPrinter *> pPrinterList;
};


#endif /* IA_PRINTER_H */



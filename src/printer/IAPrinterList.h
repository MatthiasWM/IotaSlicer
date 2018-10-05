//
//  IAPrinterList.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_LIST_H
#define IA_PRINTER_LIST_H


#include <vector>


class IAPrinter;
struct Fl_Menu_Item;
class Fl_Browser;


/**
 * Manage a list of printers.
 */
class IAPrinterList
{
public:
    IAPrinterList();
    ~IAPrinterList();
    bool add(IAPrinter *printer);
    void remove(IAPrinter *printer);
    int size() { return (int)pPrinterList.size(); }

    void generatePrototypes();
    void loadCustomPrinters(IAPrinter *(&p));
    void saveCustomPrinters();

    IAPrinter *cloneAndAdd(IAPrinter *p);
    char *makeUniqueName(char const* name) const;

    Fl_Menu_Item *createPrinterAddMenu();
    Fl_Menu_Item *createPrinterSelectMenu();
    void updatePrinterSelectMenu();
    void fillBrowserWidget(Fl_Browser*, IAPrinter *select=nullptr);

private:

    std::vector<IAPrinter *> pPrinterList;
};


#endif /* IA_PRINTER_LIST_H */



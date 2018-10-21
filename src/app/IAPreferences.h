//
//  IAPreferences.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PREFERENCES_H
#define IA_PREFERENCES_H


#include <FL/filename.H>


class Fl_Preferences;


/**
 * Manage application wide user preferences.
 */
class IAPreferences
{
public:
    IAPreferences();
    ~IAPreferences();
    void flush();

    void updateRecentfilesMenu();
    void addRecentFile(const char *filename);
    void clearRecentFileList();
    const char *printerDefinitionsPath() const;

    /** main window position, or -1 if undefined. */
    int pMainWindowX = -1;
    /** main window position, or -1 if undefined. */
    int pMainWindowY = -1;
    /** main window width. */
    int pMainWindowW = 800;
    /** main window height. */
    int pMainWindowH = 600;
    /** # of recent files to store */
    static const int pNRecentFiles = 10;
    /** store the filenames of recent files here */
    char *pRecentFile[pNRecentFiles] = { 0 };
    /** write preferences for individual printers here */
    char *pPrinterDefinitionsPath = nullptr;
};


#endif /* IA_PREFERENCES_H */



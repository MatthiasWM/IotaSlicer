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
 * User preferences.
 */
class IAPreferences
{
public:
    IAPreferences();
    ~IAPreferences();
    void flush();

    int pMainWindowX = -1;
    int pMainWindowY = -1;
    int pMainWindowW = 800;
    int pMainWindowH = 600;

    char pLastGCodeFilename[FL_PATH_MAX] = { 0 };
    char pLastLoadFilename[FL_PATH_MAX] = { 0 };

    Fl_Preferences *pPrefs = nullptr;
};


#endif /* IA_PREFERENCES_H */



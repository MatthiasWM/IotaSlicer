//
//  IAPreferences.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PREFERENCES_H
#define IA_PREFERENCES_H

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

    Fl_Preferences *pPrefs = nullptr;
};


#endif /* IA_PREFERENCES_H */



//
// IAVersioneer.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_VERSIONEER_H
#define IA_VERSIONEER_H

#include "IAGUIMain.h"

#include <FL/Fl.H>
#include "Iota.h"
#include "IASceneView.h"
#include "IAGLButton.h"
#include "IAGLRangeSlider.h"
#include <FL/Fl_File_Chooser.H>
#ifdef __APPLE__
#include <unistd.h>
#include <utime.h>
#endif
#ifdef __LINUX__
#include <unistd.h>
#include <utime.h>
#endif
#ifdef _WIN32
#include <sys/utime.h>
#endif
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *wMainWindow;
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
extern IASceneView *gSceneView;
#include <FL/Fl_Box.H>
extern IAGLRangeSlider *zRangeSlider;
extern Fl_Box *wPrinterName;
#include <FL/Fl_Tree.H>
extern Fl_Tree *wSessionSettings;
Fl_Double_Window* createIotaAppWindow();
extern Fl_Menu_Item menu_[];
#define wPrinterSelectionMenu (menu_+42)
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Button.H>
Fl_Double_Window* createIotaAboutWindow();
extern Fl_Double_Window *wVersionWindow;
#include <FL/Fl_Input.H>
extern Fl_Input *wBasePath;
extern Fl_Input *wMajor;
extern Fl_Input *wMinor;
extern Fl_Input *wBuild;
#include <FL/Fl_Input_Choice.H>
extern Fl_Input_Choice *wClass;
extern Fl_Button *wMajorPlus;
extern Fl_Button *wMinorPlus;
extern Fl_Button *wBuildPlus;
void loadSettings();
void saveSettings();
void applySettingsHtml(const char *name);
void applySettingsCpp(const char *name);
void applySettingsDoxyfile(const char *name);
void applySettings();
void gitTag();
Fl_Double_Window* createVersionWindow();
extern Fl_Menu_Item menu_wClass[];


class IAVersioneer : public IAVersioneerDialog
{
public:
    IAVersioneer();
    virtual ~IAVersioneer();
    void show();

protected:
    virtual void setProjectPath() override;
    virtual void selectProjectPath() override;
    virtual void majorPlus() override;
    virtual void minorPlus() override;
    virtual void buildPlus() override;
    virtual void saveSettings() override;
    virtual void gitTag() override;
    virtual void applySettings() override;

    void loadSettings();
    void applySettingsDoxyfile(const char *name);
    void applySettingsCpp(const char *name);
    void applySettingsHtml(const char *name);
    void touch(const char *name);
    void updatePlatformFiles();

    void system(const char *);

    Fl_Double_Window *pDialog = nullptr;
};

#endif /* IA_VERSIONEER_H */

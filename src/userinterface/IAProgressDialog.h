//
//  IAProgressDialog.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PROGRESS_DIALOG_H
#define IA_PROGRESS_DIALOG_H


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Progress.H>


class IAProgressDialog
{
public:
    static void show(const char *title, const char *text = nullptr);
    static void setText(const char *text);
    static bool update(double percent, ...);
    static void hide();

private:
    static void cb_Abort(Fl_Button*, void*);
    static void createProgressDialog();
    
    static void hideProgressDialog();
    static bool updateProgressDialog();

    static char *pTitle;
    static char *pText;
    static Fl_Double_Window *wDialog;
    static Fl_Progress *wProgress;
    static Fl_Box *wText;
    static bool pCanceled;
};





#endif /* IA_PROGRESS_DIALOG_H */

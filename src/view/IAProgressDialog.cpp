//
//  IAProgressDialog.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAProgressDialog.h"
#include "IAGUIMain.h"

#include <stdarg.h>


char *IAProgressDialog::pTitle = nullptr;
char *IAProgressDialog::pText = nullptr;
Fl_Double_Window *IAProgressDialog::wDialog = nullptr;
Fl_Progress *IAProgressDialog::wProgress = nullptr;
Fl_Box *IAProgressDialog::wText = nullptr;
bool IAProgressDialog::pCanceled = false;


void IAProgressDialog::cb_Abort(Fl_Button*, void*)
{
    pCanceled = true;
}


void IAProgressDialog::createProgressDialog()
{
    if (wDialog) return;

    wDialog = new Fl_Double_Window(436, 102, "Progress");
    wProgress = new Fl_Progress(24, 37, 388, 20);
    Fl_Button* o = new Fl_Button(320, 67, 92, 24, "Abort");
    o->callback((Fl_Callback*)cb_Abort);
    wText = new Fl_Box(24, 9, 388, 28, "Creating slice %d of %d.");
    wText->labelsize(12);
    wDialog->set_modal();
    wDialog->end();
}


/**
 * Show the progress dialog box.
 *
 * The dialog has a title text in the window decoration, a descriptive text,
 * a progress bar, and a "cancel" button for the user.
 *
 * The descriptive text will not be shown until IAProgressDialog::update
 * is called.
 *
 * \param title a static dialog title, no need to retain or manage
 * \param text a descriptive text, formatted in the style of \e printf, no
 *      need to retain or manage.
 */
void IAProgressDialog::show(const char *title, const char *text) {
    if (!wDialog) {
        createProgressDialog();
    }
    pCanceled = false;
    setText(text);
    wDialog->copy_label(title);
    wText->label("0%");
    wProgress->value(0);
    int x = wMainWindow->x() + (wMainWindow->w()-wDialog->w())/2;
    int y = wMainWindow->y() + (wMainWindow->h()-wDialog->h())/2;
    wDialog->position(x, y);
    wDialog->show();
}


/**
 * Change the descriptive text.
 *
 * The descriptive text change will not be shown until IAProgressDialog::update
 * is called.
 *
 * \param text a descriptive text, formatted in the style of \e printf, no
 *      need to retain or manage.
 */
void IAProgressDialog::setText(const char *text)
{
    if (pText)
        ::free((void*)pText);
    if (text)
        pText = strdup(text);
    else
        pText = strdup("");
}


/**
 * Update the progress in this dialog.
 *
 * \param percent update the pogress indicator, [0..100]
 * \param ... add varaibles that correspond to the formatting in the descriptive
 *      text. For example, if the text is "Processing line %d in file %s...",
 *      this call should be IAProgressDialog::update(10.0, 513, "test.txt").
 *
 * \return true, if the user clicked \e cancel any time while the progress
 *      dialog was up.
 */
bool IAProgressDialog::update(double percent, ...)
{
    char buf[2048];

    va_list va;
    va_start(va, percent);
    vsnprintf(buf, 2047, pText, va);
    va_end(va);

    wText->copy_label(buf);
    wProgress->value((float)percent);

    Fl::check();
    Fl::check();
    Fl::check();
    return pCanceled;
}


/**
 * Hide the progress dialog.
 */
void IAProgressDialog::hide()
{
    if (wDialog)
        wDialog->hide();
}


static bool progressDialogCanceled;

Fl_Double_Window *wProgressDialog=(Fl_Double_Window *)0;

Fl_Progress *wProgressValue=(Fl_Progress *)0;


Fl_Box *wProgressText=(Fl_Box *)0;


void hideProgressDialog() {
    wProgressDialog->hide();
}

/**
 Returns true, if the user interrupted the action.
 */
bool updateProgressDialog() {
    Fl::check();
    return progressDialogCanceled;
}


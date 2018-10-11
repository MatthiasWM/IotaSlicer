//
//  IAFLTreeItemFloat.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_FL_TREE_ITEM_FLOAT_H
#define IA_FL_TREE_ITEM_FLOAT_H


#include "printer/IASetting.h"


#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Float_Input.H>



class IAFLTreeItemFloat : public Fl_Group
{
public:
    IAFLTreeItemFloat(IASetting::Type t, int w, const char *label=nullptr)
    :   Fl_Group(0, 0, w, t==IASetting::kSetting?13:15) {
        box(FL_FLAT_BOX);
        begin();
        pLabel = new Fl_Box(0, 0, w/2-4, h());
        pLabel->labelsize(h()-2);
        pLabel->label(label);
        pInput = new Fl_Float_Input(w/2, 0, 65, h());
        pInput->textsize(h()-2);
        pInput->labelsize(h()-2);
        pInput->align(FL_ALIGN_RIGHT);
        pInput->callback((Fl_Callback*)choice_cb);
        if (t==IASetting::kSetting) {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
        } else {
            pLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
            color(parent()->color());
            pLabel->color(parent()->color());
            pInput->color(parent()->color());
        }
        end();
    }
    ~IAFLTreeItemFloat() { }
    double value() { return atof(pInput->value()); }
    void value(double v) {
        char buf[80];
        snprintf(buf, 80, "%.2f", v);
        pInput->value(buf);
    }
    static void choice_cb(Fl_Float_Input *w, void *u) {
        w->parent()->do_callback();
    }
    Fl_Box *pLabel = nullptr;
    Fl_Float_Input *pInput = nullptr;
};


#endif /* IA_FL_TREE_ITEM_FLOAT_H */



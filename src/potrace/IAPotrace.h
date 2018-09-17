//
//  IAPotrace.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_POTRACE_H
#define IA_POTRACE_H


class IAFramebuffer;
class IAToolpathList;


int potrace(IAFramebuffer *framebuffer, IAToolpathList *toolpath, double z);


#endif /* IA_POTRACE_H */

//
//  IAError.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_ERROR_H
#define IA_ERROR_H

#include <stdio.h>
#include <memory>


/**
 * This class manages errors that are presented to the user.
 */
class IAError
{
public:
    /**
     * List of errors that the user may encounter.
     */
    typedef enum {
        NoError = 0,
        CantOpenFile_STR_BSD,
        UnknownFileType_STR,
        FileContentCorrupt_STR,
        OpenGLFeatureNotSupported_STR,
    } Error;

    static void clear();
    static void set(const char *loc, Error err, const char *str=nullptr);
    static bool hadError();

    /** Last error, that occured since clearError().
     \return error code. */
    static Error error() { return pError; }
    static void showDialog();

private:
    /// user definable string explaining the details of an error
    static const char *pErrorString;
    /// user definable string explaining the function that caused an error
    static const char *pErrorLocation;
    /// current error condition
    static Error pError;
    /// system specific error number of the call that was markes by an error
    static int pErrorBSD;
    /// list of error messages
    static const char *kErrorMessage[];
};


#endif /* IA_ERROR_H */

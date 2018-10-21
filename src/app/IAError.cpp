//
//  IAError.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAError.h"

#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>

#include <errno.h>

#ifdef _WIN32
#include <ShlObj.h>
#endif


#define HDR "Error in Iota Slicer, %s\n\n"


/// user definable string explaining the details of an error
const char *IAError::pErrorString = nullptr;

/// user definable string explaining the function that caused an error
const char *IAError::pErrorLocation = nullptr;

/// current error condition
IAError::Error IAError::pError = IAError::NoError;

/// system specific error number of the call that was markes by an error
int IAError::pErrorBSD = 0;

/**
 * Error messages for error code from the Error enum class.
 *
 * These strings are called with the varargs
 *  1) function: a human readable location where this error occured.
 *  2) a caller provided string, often a file name
 *  3) the cause of an error as a text returned by the operating system
 */
const char *IAError::kErrorMessage[] =
{
    // NoError
    HDR"No error.",
    // CantOpenFile_STR_BSD
    HDR"Can't open file \"%s\":\n%s",
    // UnknownFileType_STR
    HDR"Unknown and unsupported file type:\n\"%s\"",
    // FileContentCorrupt_STR
    HDR"There seems to be an error inside this file:\n\"%s\"",
    // OpenGLFeatureNotSupported_STR
    HDR"Required OpenGL graphics feature not suported:\n\"%s\"",
};


/**
 * Clear all information about previous errors.
 *
 * clearError() and showError() are used like brackets in a high level
 * call from the user interface.
 *
 * A typical user interaction starts with clearError(),
 * followed by a function that may create an error,
 * and finalized by showLastError(), which will present the error to the user,
 * only if one occured.
 *
 * \see IAIota::showLastError(), IAIota::hadError()
 */
void IAError::clear()
{
    pError = Error::NoError;
    pErrorBSD = 0;
    pErrorString = nullptr;
}


/**
 * Set an error code that may or may not be shown to the user later.
 *
 * Errors most be meaningful to the user first and foremost!
 *
 * \param[in] loc the error location. This description should be useful to the user,
 *        not the programmer
 * \param[in] err the actual error message. Error messages ending in _BSD will store
 *        the 'errno' at the time of this call.
 * \param[in] str error messages ending in _STR require additonal text to print the
 *        error message, usually a file name
 */
void IAError::set(const char *loc, Error err, const char *str)
{
    pErrorLocation = loc;
    pError = err;
    pErrorBSD = errno;
    if (pErrorString)
        ::free((void*)pErrorString);
    pErrorString = str ? strdup(str) : nullptr;
}


/**
 * Return true, if setError() was called since the last call to clearError().
 *
 * \return true if there was an error since the last call to clearError()
 */
bool IAError::hadError()
{
    return (pError!=Error::NoError);
}


/**
 * Show an alert box with the text of the last error, registered by setError().
 */
void IAError::showDialog()
{
    if (hadError()) {
        fl_alert(kErrorMessage[(size_t)pError], pErrorLocation, pErrorString, strerror(pErrorBSD));
    }
}


//
//  IotaMacros.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IOTA_MACROS_H
#define IOTA_MACROS_H


#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED(text) __attribute__((deprecated(text)))
// void fn() __attribute__ ((deprecated("text"))) { ... }
#elif defined(_MSC_VER)
#define DEPRECATED(text) __declspec(deprecated(text))
// __declspec(deprecated("** this is a deprecated function **")) void fn() { ... }
#else
#error define this
#define DEPRECATED [[deprecated]]
#define DEPRECATED
#endif


#endif /* IOTA_MACROS_H */

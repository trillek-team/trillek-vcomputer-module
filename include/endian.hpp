#pragma once
#ifndef __ENDIAN_HPP__
#define __ENDIAN_HPP__ 1
/*
 * ByteOrder - portable tools to determine the byte order (endianness),
 * swap the order of bytes in integer value, and convert values between host
 * and little-/big-endian byte order.
 *
 * http://projects.malcom.pl/libs/byteorder.xhtml
 * http://github.com/malcom/ByteOrder
 *
 * Copyright (C) 2013 Marcin 'Malcom' Malich <me@malcom.pl>
 *
 * Released under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

// Avoid clash with SDL2
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN       1234
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN          4321
#endif

#define LITTLE_ENDIAN_NAME  "litte endian"
#define BIG_ENDIAN_NAME     "big endian"

#ifdef EMSCRIPTEN
// With emscripte we use the Big Endian code as not abuse of endianess
#define BYTE_ORDER BIG_ENDIAN
#endif

#ifndef BYTE_ORDER

// based on boost/detail/endian.hpp
#if defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN) || \
  defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__) || \
defined(_STLP_LITTLE_ENDIAN) && !defined(_STLP_BIG_ENDIAN)

#define BYTE_ORDER LITTLE_ENDIAN

#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN) || \
  defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__) || \
defined(_STLP_BIG_ENDIAN) && !defined(_STLP_LITTLE_ENDIAN)

#define BYTE_ORDER BIG_ENDIAN

#elif defined(__sparc) || defined(__sparc__) || \
  defined(_POWER) || defined(__powerpc__) || \
defined(__ppc__) || defined(__hpux) || defined(__hppa) || \
defined(_MIPSEB) || defined(__s390__)

#define BYTE_ORDER BIG_ENDIAN

#elif defined(__i386__) || defined(__alpha__) || \
  defined(__ia64) || defined(__ia64__) || \
defined(_M_IX86) || defined(_M_IA64) || \
defined(_M_ALPHA) || defined(__amd64) || \
defined(__amd64__) || defined(_M_AMD64) || \
defined(__x86_64) || defined(__x86_64__) || \
defined(_M_X64) || defined(__bfin__)

#define BYTE_ORDER LITTLE_ENDIAN

#else
#error Unknown machine endianness detected.
#endif

#endif // BYTE_ORDER


#ifndef BYTE_ORDER_NAME

#if BYTE_ORDER == LITTLE_ENDIAN
#define BYTE_ORDER_NAME LITTLE_ENDIAN_NAME
#elif BYTE_ORDER == BIG_ENDIAN
#define BYTE_ORDER_NAME BIG_ENDIAN_NAME
#else
#error Unknown machine endianness detected.
#endif

#endif // BYTE_ORDER_NAM

#endif // __ENDIAN_HPP__

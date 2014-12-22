/**
 * \brief       Virtual Computer macro for Windows DLLs
 * \file        vc_dll.hpp
 * \copyright   The MIT License (MIT)
 *
 * A little macro to put the necesary Visual Studio stff to generate correct
 * DLLs
 */

#ifndef __VC_DLL_HPP_
#define __VC_DLL_HPP_ 1

#if defined(_MSC_VER)

#if defined DLL_EXPORT
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif

#else
#define DECLDIR
#endif // if defined(_MSC_VER)

#endif // __VC_DLL_HPP_


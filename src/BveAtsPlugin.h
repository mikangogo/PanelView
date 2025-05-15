#ifndef BVE_ATS_PLUGIN_H_INCLUDED
#define BVE_ATS_PLUGIN_H_INCLUDED

#if defined(__cplusplus)
extern "C"
{
#endif

#if !defined(WINAPI)
#if defined(_MSC_VER)
#define WINAPI __stdcall
#else
#define WINAPI
#endif
#endif

#if !defined(ATS_API)
#if defined(_MSC_VER)
    // HACK: A simple and convenient method that can export both x86 and x64 architectures.
    // See also: https://docs.microsoft.com/ja-jp/office/client-developer/excel/developing-dlls
#if _MSC_VER > 1200 // Later than Visual Studio 6.0
#define EXPORT comment(linker, "/EXPORT:"__FUNCTION__"="__FUNCDNAME__)
#define ATS_API
#else // Cannot use this way of exporting functions.
#define EXPORT
#define ATS_API __declspec(dllexport)
#endif // else need to use DEF file or __declspec(dllexport)
#define EXPORT_THIS_FUNCTION __pragma(EXPORT)
#else
#define ATS_API __attribute__((visibility("default")))
#define EXPORT_THIS_FUNCTION
#endif
#endif

#include "atsplugin.h"

#if defined(__cplusplus)
}
#endif

#endif  // BVE_ATS_PLUGIN_H_INCLUDED
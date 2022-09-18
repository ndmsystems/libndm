#ifndef __NDM_VISIBILITY__
#define __NDM_VISIBILITY__

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef BUILDING_DLL
#ifdef __GNUC__
#define __NDM_VISIBILITY_EXPORT__ __attribute__((dllexport))
#else
#define __NDM_VISIBILITY_EXPORT__ __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define __NDM_VISIBILITY_EXPORT__ __attribute__((dllimport))
#else
#define __NDM_VISIBILITY_EXPORT__ __declspec(dllimport)
#endif
#define __NDM_VISIBILITY_HIDDEN__
#endif
#else
#if __GNUC__ >= 4
#define __NDM_VISIBILITY_EXPORT__ __attribute__ ((visibility("default")))
#define __NDM_VISIBILITY_HIDDEN__ __attribute__ ((visibility("hidden")))
#else
#define __NDM_VISIBILITY_EXPORT__
#define __NDM_VISIBILITY_HIDDEN__
#endif
#endif

#endif /* __NDM_VISIBILITY__ */


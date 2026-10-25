// SPDX-License-Identifier: MIT
/* Public WinCompat UxTheme consumer declarations. */
#ifndef RIN_SDK_COMPAT_UXTHEME_ABI_H
#define RIN_SDK_COMPAT_UXTHEME_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* These fixed-width aliases match the RinOS WinCompat export surface without
 * colliding with types from a host Windows SDK. HWND is an opaque pointer in
 * the product ABI; HRESULT, DWORD, and the attribute enum are 32-bit values. */
typedef void* RinWinCompatHwnd;
typedef int32_t RinWinCompatHresult;
typedef uint32_t RinWinCompatDword;
typedef int32_t RinWinCompatWindowThemeAttributeType;

#define RIN_WINCOMPAT_UXTHEME_DLL_NAME "uxtheme.dll"
#define RIN_WINCOMPAT_UXTHEME_SET_WINDOW_THEME_ATTRIBUTE_EXPORT \
    "SetWindowThemeAttribute"

#define RIN_WINCOMPAT_WTA_NONCLIENT INT32_C(1)

#define RIN_WINCOMPAT_WTNCA_NODRAWCAPTION UINT32_C(0x00000001)
#define RIN_WINCOMPAT_WTNCA_NODRAWICON    UINT32_C(0x00000002)
#define RIN_WINCOMPAT_WTNCA_NOSYSMENU     UINT32_C(0x00000004)
#define RIN_WINCOMPAT_WTNCA_NOMIRRORHELP  UINT32_C(0x00000008)
#define RIN_WINCOMPAT_WTNCA_VALIDBITS     UINT32_C(0x0000000f)

typedef struct RinWinCompatWtaOptions {
    RinWinCompatDword dwFlags;
    RinWinCompatDword dwMask;
} RinWinCompatWtaOptions;

/* The exported API uses the product's default C calling convention. Resolve
 * the named export from uxtheme.dll and call it through this function type.
 * dwMask selects which WTNCA bits are updated; dwFlags supplies their values. */
typedef RinWinCompatHresult RinWinCompatSetWindowThemeAttribute(
    RinWinCompatHwnd window,
    RinWinCompatWindowThemeAttributeType attribute,
    void* attributes,
    RinWinCompatDword attribute_size);
typedef RinWinCompatSetWindowThemeAttribute*
    RinWinCompatSetWindowThemeAttributeFn;

#if defined(__cplusplus)
static_assert(sizeof(RinWinCompatHwnd) == sizeof(void*),
              "WinCompat HWND ABI drift");
static_assert(sizeof(RinWinCompatHresult) == 4u &&
                  sizeof(RinWinCompatDword) == 4u &&
                  sizeof(RinWinCompatWindowThemeAttributeType) == 4u,
              "WinCompat UxTheme scalar ABI drift");
static_assert(sizeof(RinWinCompatWtaOptions) == 8u &&
                  offsetof(RinWinCompatWtaOptions, dwFlags) == 0u &&
                  offsetof(RinWinCompatWtaOptions, dwMask) == 4u,
              "WinCompat WTA_OPTIONS ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinWinCompatHwnd) == sizeof(void*),
               "WinCompat HWND ABI drift");
_Static_assert(sizeof(RinWinCompatHresult) == 4u &&
                   sizeof(RinWinCompatDword) == 4u &&
                   sizeof(RinWinCompatWindowThemeAttributeType) == 4u,
               "WinCompat UxTheme scalar ABI drift");
_Static_assert(sizeof(RinWinCompatWtaOptions) == 8u &&
                   offsetof(RinWinCompatWtaOptions, dwFlags) == 0u &&
                   offsetof(RinWinCompatWtaOptions, dwMask) == 4u,
               "WinCompat WTA_OPTIONS ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_COMPAT_UXTHEME_ABI_H */

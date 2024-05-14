#include "gui/dpi.hpp"

#ifdef _MSC_VER

#include <Windows.h>

#else

#include <X11/Xlib.h>

#endif

// ------------------------------------------------------------------------------------------------
float GetDpiScale()
{
#ifdef _MSC_VER
    const auto hdc = GetDC(nullptr);
    const int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(nullptr, hdc);
#else
    auto* display = XOpenDisplay(nullptr);
    int screen = DefaultScreen(display);
    int dpi = DisplayWidth(display, screen) * 25.4f / DisplayWidthMM(display, screen);
    XCloseDisplay(display);
#endif
    return static_cast<float>(dpi) / 96.0f;
}

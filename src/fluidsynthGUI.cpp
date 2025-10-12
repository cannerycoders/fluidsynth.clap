#include "fluidsynthPlugin.h"

#include <clap/helpers/host-proxy.hxx>
#include <iostream>
#include <cstring>

bool 
FluidsynthPlugin::guiIsApiSupported(const char *api, bool isFloating) noexcept 
{
    if(!strcmp(api, "webview")) return true;
    else return false;
}

bool
FluidsynthPlugin::guiGetPreferredApi(const char **api, bool *is_floating) noexcept 
{
    *api = "webview";
    *is_floating = false;
    return true;
}

/* request a gui for this instance...  Since we only operate in
 * non-floating mode, the real work won't happen 'til setParent.
 * We do this because host squirrels some info into clap_window.
 */
bool
FluidsynthPlugin::guiCreate(const char *api, bool isFloating) noexcept 
{
    if(!strcmp(api, "webview")) 
        return true;
    else 
        return false;
}

void
FluidsynthPlugin::guiDestroy() noexcept 
{
}

bool
FluidsynthPlugin::guiSetParent(const clap_window *window) noexcept 
{
    return true;;
}

bool
FluidsynthPlugin::guiShow() noexcept 
{
    return true;
}

bool
FluidsynthPlugin::guiHide() noexcept 
{
    return true;
}

void
FluidsynthPlugin::guiSuggestTitle(const char *title) noexcept 
{
    // would be nice to deliver this to gui, preferred over instance 10000
}

bool
FluidsynthPlugin::guiSetTransient(const clap_window *window) noexcept 
{
    return false;
}

bool
FluidsynthPlugin::guiSetScale(double scale) noexcept 
{
    return false;
}

bool
FluidsynthPlugin::guiGetSize(uint32_t *width, uint32_t *height) noexcept 
{
    *width = 0;
    *height = 0;
    return true;
}

bool
FluidsynthPlugin::guiCanResize() const noexcept 
{
    return false;
}

bool
FluidsynthPlugin::guiGetResizeHints(clap_gui_resize_hints_t *hints) noexcept 
{
    return true;
}

bool
FluidsynthPlugin::guiAdjustSize(uint32_t *width, uint32_t *height) noexcept 
{
    return true;
}

bool
FluidsynthPlugin::guiSetSize(uint32_t width, uint32_t height) noexcept 
{
    return true;
}

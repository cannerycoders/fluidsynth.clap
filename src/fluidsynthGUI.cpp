#include "fluidsynthPlugin.h"
#include <iostream>

bool 
FluidsynthPlugin::guiIsApiSupported(const char *api, bool isFloating) noexcept 
{
    if(isFloating || !strcmp(api, "webview")) 
        return false;
    else
        return true;
}

bool
FluidsynthPlugin::guiGetPreferredApi(const char **api, bool *is_floating) noexcept 
{
    return false; // no preferred API atm
}

bool
FluidsynthPlugin::guiCreate(const char *api, bool isFloating) noexcept 
{
    #if 0
    // window is created by host and passed to us (unless we want isFloating)
    assert(m_window == nullptr);
    m_window = new choc::ui::DesktopWindow({100, 100, 800, 600});
    m_window->setWindowTitle("FluidSynth");
    m_window->setResizable(true);
    m_window->setMinimumSize(300, 300);
    m_window->setMaximumSize(1500, 1200);
    m_window->windowClosed = [this] 
    { 
        m_window->setVisible(false);
    };
    #endif
    m_webview = new choc::ui::WebView();
    return true;
}

void
FluidsynthPlugin::guiDestroy() noexcept 
{
    // m_window->setVisible(false);
    // xxx: delete m_window;
}

bool
FluidsynthPlugin::guiSetScale(double scale) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: implement guiSetScale");
    return false;
}

bool
FluidsynthPlugin::guiShow() noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiShow");
    // m_window->setVisible(false);
    return false;
}
bool
FluidsynthPlugin::guiHide() noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiHide");
    return false;
}
bool
FluidsynthPlugin::guiGetSize(uint32_t *width, uint32_t *height) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiGetSize");
    *width = 512;
    *height = 512;
    return true;
}

bool
FluidsynthPlugin::guiCanResize() const noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiCanResize");
    return true;
}

bool
FluidsynthPlugin::guiGetResizeHints(clap_gui_resize_hints_t *hints) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiGetResizeHints");
    return false;
}

bool
FluidsynthPlugin::guiAdjustSize(uint32_t *width, uint32_t *height) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiAdjustSize");
    *width = m_guiSize[0];
    *height = m_guiSize[1];
    return true;
}

bool
FluidsynthPlugin::guiSetSize(uint32_t width, uint32_t height) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: setSize");
    m_guiSize[0] = width;
    m_guiSize[1] = height;
    return true;
}

void
FluidsynthPlugin::guiSuggestTitle(const char *title) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiSuggestTitle");
}

bool
FluidsynthPlugin::guiSetParent(const clap_window *window) noexcept 
{
    bool handled = true;
    void *viewHandle = m_webview->getViewHandle();
#ifdef _WIN32
    HWND hwnd = (HWND) window->win32;
    HWND child = (HWND) viewHandle;
    auto flags = GetWindowLongPtr (child, -16);
    flags = (flags & ~(decltype (flags)) WS_POPUP) | (decltype (flags)) WS_CHILD;
    SetWindowLongPtr(child, -16, flags);
    SetParent(child, hwnd);
    ShowWindow(child, SW_SHOW);
    m_webview->navigate("https://cannerycoders.com");
    std::cerr << "guiSetParent\n";
#else
    #error "unimplemented platform"
    handled = false;
#endif
    return handled;
}

bool
FluidsynthPlugin::guiSetTransient(const clap_window *window) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiSetTransient");
    return false;
}
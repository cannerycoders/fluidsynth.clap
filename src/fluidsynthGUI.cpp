#include "fluidsynthPlugin.h"

#include <clap/helpers/host-proxy.hxx>
#include <iostream>

bool 
FluidsynthPlugin::guiIsApiSupported(const char *api, bool isFloating) noexcept 
{
    if(isFloating)
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
    m_guiSize[0] = 512;
    m_guiSize[1] = 512;
    using Resource = choc::ui::WebView::Options::Resource;
    using Path = choc::ui::WebView::Options::Path;
    m_webviewOptions.fetchResource = [this](Path const  &path) -> std::optional<Resource>
    {
        return this->GetResource(path);
    };
    m_webview = new choc::ui::WebView(m_webviewOptions);
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
    return true;
}

bool
FluidsynthPlugin::guiGetResizeHints(clap_gui_resize_hints_t *hints) noexcept 
{
    // _host.log(CLAP_LOG_INFO, "fluidsynth: guiGetResizeHints");
    return false;
}

bool
FluidsynthPlugin::guiAdjustSize(uint32_t *width, uint32_t *height) noexcept 
{
    char buf[128];
    sprintf(buf, "fluidsynth: adjustSize %d %d -> %d %d", 
            *width, *height, m_guiSize[0], m_guiSize[1]);
    _host.log(CLAP_LOG_INFO, buf);
    *width = m_guiSize[0];
    *height = m_guiSize[1];
    return true;
}

bool
FluidsynthPlugin::guiSetSize(uint32_t width, uint32_t height) noexcept 
{
    char buf[128];
    sprintf(buf, "fluidsynth: setSize %d %d", width, height);
    _host.log(CLAP_LOG_INFO, buf);
    m_guiSize[0] = width;
    m_guiSize[1] = height;
    if(_host.canUseGui())
        _host.guiRequestResize(width, height);
    else
        _host.log(CLAP_LOG_WARNING, "Host doesn't support gui extension.");
    return true;
}

void
FluidsynthPlugin::guiSuggestTitle(const char *title) noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiSuggestTitle");
}

/**
 * install our webview into host-provided window
 */
bool
FluidsynthPlugin::guiSetParent(const clap_window *window) noexcept 
{
    bool handled = true;
    void *viewHandle = m_webview->getViewHandle();
#ifdef _WIN32
    RECT r;
    HWND hwnd = (HWND) window->win32;
    HWND newchild = (HWND) viewHandle;
    auto flags = GetWindowLongPtr(newchild, -16);
    flags = (flags & ~(decltype (flags)) WS_POPUP) | (decltype (flags)) WS_CHILD;
    SetWindowLongPtr(newchild, -16, flags);
    GetClientRect(hwnd, &r);
    SetParent(newchild, hwnd);
    SetWindowPos(newchild, nullptr, r.left, r.top, r.right-r.left, r.bottom-r.top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
    ShowWindow(newchild, SW_SHOW); // we're visible independent of our parent
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

static char const *s_index = R"(
<!doctype html>
<html lang="en">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<style>
html {
    background-color: #111;
}
body {
    background-color: #111;
    border-width: 1px;
    border: solid;
    border-color: #080808;
    color: #888;
    height: 100vh;
    width: 100%;
    margin: 0px;
    box-sizing: border-box;
    padding: 10px;
}
</style>
<body>
    <H3>FluidSynth.clap</H3>
</body>
</html>
)";
std::optional<choc::ui::WebView::Options::Resource> 
FluidsynthPlugin::GetResource(choc::ui::WebView::Options::Path const &path)
{
    if(path == "/" || path == "/index.html")
    {
        choc::ui::WebView::Options::Resource rez; // rez.data vector<unit8_t>
        rez.mimeType = "text/html";
        for(char const *s = s_index; *s != '\0'; s++)
            rez.data.push_back(*s);
        return rez;
    }
    else
    {
        char buf[1024];
        sprintf(buf, "fluidsynth.getrez %s", path.c_str());
        _host.log(CLAP_LOG_INFO, buf);
        return std::nullopt;
    }
#if 0
    std::filesystem::path wwwfile = m_wwwDir;
    if(path == "/")
        wwwfile /= "index.html";
    else
    {
        if(path[0] == '/')
        {
            // workaround this fact: "/foo/bar" / "/img" => "/img"
            wwwfile /= std::string(path.begin()+1, path.end());
        }
        else
            wwwfile /= path;
    }
    std::string np = Workspace::FixupPath(wwwfile); // can't have mix of \ & /
    if(std::filesystem::exists(np))
    {
        std::ifstream ifs(np, std::ios::binary);
        if(ifs.good())
        {
            choc::ui::WebView::Options::Resource rez;
            rez.data.assign((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
            auto ext = wwwfile.extension().generic_string();
            if(ext == ".html")
                rez.mimeType = "text/html";
            else
            if(ext == ".css")
                rez.mimeType = "text/css";
            else
            if(ext == ".js")
                rez.mimeType = "text/javascript";
            else
            if(ext == ".json")
                rez.mimeType = "application/json";
            else
            if(ext == ".md")
                rez.mimeType = "text/markdown";
            else
            if(ext == ".ttf")
                rez.mimeType = "font/ttf";
            else
            if(ext == ".woff")
                rez.mimeType = "font/woff";
            else
            if(ext == ".png")
                rez.mimeType = "image/png";
            else
            if(ext == ".ico")
                rez.mimeType = "image/x-icon";
            else
            if(ext == ".gif")
                rez.mimeType = "image/gif";
            else
            if(ext == ".jpg")
                rez.mimeType = "image/jpg";
            else
            if(ext == ".raw")
                rez.mimeType = "application/octet-stream";
            else
            {
                LogDebug(LogFormat("Unknown file extension %s", ext.c_str()));
                rez.mimeType = "application/octet-stream";
            }
            return rez;
        }
    }
    else
        LogWarning(LogFormat("Missing www resource %s", np.c_str()));
    return std::nullopt;
#endif
}
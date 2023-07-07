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
    if(m_verbosity)
        _host.log(CLAP_LOG_INFO, "fluidsynth: guiShow");
    // m_window->setVisible(false);
    return false;
}
bool
FluidsynthPlugin::guiHide() noexcept 
{
    if(m_verbosity)
        _host.log(CLAP_LOG_INFO, "fluidsynth: guiHide");
    return false;
}
bool
FluidsynthPlugin::guiGetSize(uint32_t *width, uint32_t *height) noexcept 
{
    if(m_verbosity)
        _host.log(CLAP_LOG_INFO, "fluidsynth: guiGetSize");
    *width = 512;
    *height = 600;
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
    hints->can_resize_horizontally = false;
    hints->can_resize_vertically = false;
    return true;
}

bool
FluidsynthPlugin::guiAdjustSize(uint32_t *width, uint32_t *height) noexcept 
{
    if(m_verbosity)
    {
        char buf[128];
        sprintf(buf, "fluidsynth: adjustSize %d %d -> %d %d", 
                *width, *height, m_guiSize[0], m_guiSize[1]);
        _host.log(CLAP_LOG_INFO, buf);
    }
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
    //_host.log(CLAP_LOG_INFO, "fluidsynth: guiSetTransient");
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
input {
    color: #ccc;
    background-color: #222;
    border-width: thin;
}
#filepath {
    width: 100%;
}
.Group {
    background-color: #000;
    border: solid;
    border-width: thin;
    border-radius: 5px;
    border-color: #404070;
    padding: 10px;
    margin-bottom: 10px;
}
.Group > div {
    margin-bottom: 5px;
}
h3,h4 {
    color: #8080a0;
}
td,th {
    text-align: left;
}
h3 > a {
    font-size: .8em;
    color: #8080FF;
}
.Title {
    font-size: 1.15em;
    color: #606090;
}
.Label {
    display: inline-block;
    width: 8em;
    padding-left: 5px;
    padding-right: 10px;
}
#voicelist {
    overflow-y: auto;
    height: 15em;
    padding-left: 10px;
    padding-right: 10px;
}
</style>
<body >
    <H3>FluidSynth.clap  
        <a href="https://github.com/cannerycoders/fluidsynth.clap">github</a> |
        <a href="http://fluidsynth.org">fluidsynth.org</a>
    </H3>
    <div class="Group">
        <!-- just want a file-pathname widget, no drop-zone etc -->
        <div class="Title">Sound Font</div>
        <div><input id="filepath" value="default.sf2"></div>
    </div>
    <div class="Group">
        <div class="Title">Parameters</div>
        <div><div class="Label">Gain</div><input id="gain" type="number" value="1" min="0" max="8" step=".1"></div>
        <div><div class="Label">Program 0</div><input id="prog0" type="number" value="0" min="0" max="127"></div>
        <div><div class="Label">Bank 0</div><input id="bank0" type="number" value="0" min="0" max="127"></div>
    </div>
    <div class="Group">
        <div class="Title">Programs</div>
        <div id="voicelist">
        </div>
    </div>

<!------------------------------------------------------------------>
<script>
function initBindings()
{
    let sfel = document.querySelector("#filepath");
    sfel.addEventListener("dragover", (evt) =>
    {
        // preventDefault to allow drop
        evt.preventDefault();
    });
    sfel.addEventListener("drop", (evt) =>
    {
        evt.preventDefault(); // prevent default action (open as link for some elements)
        for(const item of evt.dataTransfer.items) 
        { 
            if(item.kind === "string" && item.type.match("^text/plain"))
            {
                item.getAsString((s) =>
                {
                    sfel.value = s;
                    tellHost("setparam", sfel.id, s);
                });
            }
            else
            if(item.kind === "file")
            {
                const file = item.getAsFile();
                sfel.value = file.name;
                tellHost("setparam", sfel.id, file.name);
            }
            else
                tellHost("log", `can't drop ${item.kind}`);
        }
    });
    for(let el of document.querySelectorAll("input"))
    {
        el.onchange = (evt) =>
        {
            // executes in host's mainthread
            tellHost("setparam", evt.target.id, event.target.value);
        };
    }
    tellHost("dbg", "bindings installed");
};
function updateAppData(key)
{
    if(key == "voices")
    {
        getAppData(key).then((data) =>
        {
            updateVoices(data);
        });
    }
    else
    {
        console.warn("Unimplemented appdata " + key);
    }
};
function updateVoices(json)
{
    tellHost("dbg", "updateVoices");
    let el = document.querySelector("#voicelist");
    try
    {
        let o = JSON.parse(json);
        let html = [];
        html.push("<table id-'voicetable'>");
        html.push("<tr><th>Bank</th><th>Prog</th><th>Name</th></tr>");
        for(let p of o) // an array of obj, last is {}
        {
            if(!p.nm) break;
            html.push(`<tr><td>${p.b}</td><td>${p.p}</td><td>${p.nm}</td></tr>\n`);
        }
        html.push("</table>");
        el.innerHTML = html.join("");
    }
    catch(err)
    {
        el.innerHTML = `<span class='ERROR'>${err}<br>${json}</span>`;
    }
}
document.addEventListener("DOMContentLoaded", initBindings);
tellHost("dbg", "script loaded");
</script>

</body>
</html>
)";

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
    m_webview->bind("tellHost", 
        [this](const choc::value::ValueView& args) -> choc::value::Value
        {
            std::string_view arg0 = args[0].getString();
            if(arg0 == "setparam")
            {
                std::string_view pname = args[1].getString();
                std::string_view pvalue = args[2].getString();
                if(pname == "filepath")
                {
                    // nb: this is a relative path so it will only work
                    // if the soundfonts are in m_pluginPresetDirs.
                    this->presetLoadFromLocation(CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                            pvalue.data(), nullptr);
                }
                else
                if(pname == "gain")
                {
                    double val = strtod(pvalue.data(), nullptr);
                    this->setParamValue(k_Gain, val);
                }
                else
                if(pname == "prog0")
                {
                    double val = strtod(pvalue.data(), nullptr);
                    this->setParamValue(k_Prog0, val);
                }
                else
                if(pname == "bank0")
                {
                    double val = strtod(pvalue.data(), nullptr);
                    this->setParamValue(k_Bank0, val);
                }
                else
                {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "fluidsynth unknown param %s ", pname.data());
                    _host.log(CLAP_LOG_WARNING, buf);
                }
                return choc::value::createInt32(0);
            }
            else
            if(arg0 == "log")
            {
                char buf[256];
                snprintf(buf, sizeof(buf), "fluidsynth %s (%d)", 
                    args[1].getString().data(), _host.isMainThread());
                _host.log(CLAP_LOG_INFO, buf);
                return choc::value::createInt32(0);
            }
            else
            if(arg0 == "dbg")
            {
                if(m_verbosity)
                {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "fluidsynth_dbg %s (%d)", 
                        args[1].getString().data(), _host.isMainThread());
                    _host.log(CLAP_LOG_INFO, buf);
                }
                return choc::value::createInt32(0);
            }
            else
            {
                std::cerr << "Unimplemented hostMsg " << arg0 << "\n";
                return choc::value::createInt32(-1);
            }
        });
    m_webview->bind("getAppData", 
        [this](const choc::value::ValueView& args) -> choc::value::Value
        {
            if(args.isArray() && args.size() == 1)
            {
                std::string_view arg0 = args[0].getString();
                if(arg0 == "voices")
                {
                    char const *data = m_voices.c_str();
                    if(data)
                        return choc::value::createString(data);
                    else
                        return choc::value::createString("");
                }
                else
                {
                    std::cerr << "Unimplemented appdata " << arg0 << "\n";
                    return choc::value::createString("");
                }
            }
            else
                return choc::value::createString("");
        }
    );
    return true;
}

void
FluidsynthPlugin::updateVoices()
{
    std::stringstream sstr;
    fluid_sfont_t* sfont = fluid_synth_get_sfont_by_id(m_synth, m_fontId);
    fluid_sfont_iteration_start(sfont);
    sstr << "[\n";
    for(fluid_preset_t* preset = fluid_sfont_iteration_next(sfont);
        preset != nullptr; preset = fluid_sfont_iteration_next(sfont)) 
    {
        int bankNum = fluid_preset_get_banknum(preset);
        int progNum = fluid_preset_get_num(preset);
        sstr << "{ \"b\":" << bankNum << ",";
        sstr << " \"p\":" << progNum <<  ",";
        sstr << " \"nm\":" << '"' << fluid_preset_get_name(preset) << "\" },\n";
    }
    sstr << "{}]";
    m_voices = sstr.str();
    std::string cmd("updateAppData(\"voices\")");
    m_webview->evaluateJavascript(cmd);
}

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
        if(path != "/favicon.ico")
        {
            char buf[1024];
            sprintf(buf, "fluidsynth.getrez %s", path.c_str());
            _host.log(CLAP_LOG_INFO, buf);
        }
        return std::nullopt;
    }
}
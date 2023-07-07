#include "fluidsynthPlugin.h"

#include <clap/helpers/host-proxy.hxx>
#include <iostream>

#define AS_TRANSIENT 0

bool 
FluidsynthPlugin::guiIsApiSupported(const char *api, bool isFloating) noexcept 
{
#if AS_TRANSIENT
    return isFloating;  // can't figure out embedding so create our own window
#else
    return !isFloating; // prefer embedding
#endif 
}

bool
FluidsynthPlugin::guiGetPreferredApi(const char **api, bool *is_floating) noexcept 
{
#if AS_TRANSIENT
    *is_floating = true;
    return true;
#else
    return false; // no preferred API atm
#endif
}

void
FluidsynthPlugin::guiDestroy() noexcept 
{
    _host.log(CLAP_LOG_INFO, "fluidsynth: implement guiDestroy");
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
    if(m_window)
    {
        m_window->setVisible(true);
        return true;
    }
    else
        return false;
}
bool
FluidsynthPlugin::guiHide() noexcept 
{
    if(m_verbosity)
        _host.log(CLAP_LOG_INFO, "fluidsynth: guiHide");
    if(m_window)
    {
        m_window->setVisible(false);
        return true;
    }
    else
        return false;
}
bool
FluidsynthPlugin::guiGetSize(uint32_t *width, uint32_t *height) noexcept 
{
    if(m_verbosity)
        _host.log(CLAP_LOG_INFO, "fluidsynth: guiGetSize");
    m_guiSize[0] = 512;
    m_guiSize[1] = 495;
    *width = m_guiSize[0];
    *height = m_guiSize[1];
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
        snprintf(buf, sizeof(buf), "fluidsynth: adjustSize %d %d -> %d %d", 
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
    snprintf(buf, sizeof(buf), "fluidsynth: setSize %d %d", width, height);
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
    char buf[1024];
    snprintf(buf, sizeof(buf), "fluidsynth transient title: %s", title);
    _host.log(CLAP_LOG_INFO, buf);
}

/**
 * install our webview into host-provided window, only happens in non-transient mode.
 */
bool
FluidsynthPlugin::guiSetParent(const clap_window *window) noexcept 
{
    bool handled = true;
#ifdef _WIN32
    void *viewHandle = m_webview->getViewHandle();
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
#elif defined(__APPLE__)
    #if AS_TRANSIENT
        assert(!"this shouldn't happen since we're operating in transient mode."); 
        // see bottom of guiCreate
    #else
    {
        // crashes, something to do with delegation? (ie: non-transient is WIP)
        void *viewHandle = m_webview->getViewHandle();
        id windowid = (id) window->cocoa;
        choc::objc::AutoReleasePool autoreleasePool;
        choc::objc::call<void>(windowid, "setContentView:", (id) viewHandle);
    }
    #endif
#else
    #error "unimplemented platform"
    handled = false;
#endif
    return handled;
}

bool
FluidsynthPlugin::guiSetTransient(const clap_window *window) noexcept 
{
#ifdef _WIN32_
    return false;
#else
    _host.log(CLAP_LOG_INFO, "fluidsynth: guiSetTransient");
    return true;
#endif
}

static char const *s_index = R"(
<!doctype html>
<html lang="en">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<style>
html {
    --scrollbarSize: 11px;
    --thumbBgdColor: rgb(67, 78, 83);
    --scrollbarBgdColor: #080808;
    --thumbDragColor: rgb(55, 85, 140);
    --titleColor: #6060A0;
    --voicelistBgd: #050505; 
    --activeBgd: #111d55;
    --hoverBgd: #151515;
    --headingColor: #40913f;
    --linkColor: #8080FF;
    --linkWeight: normal;
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
    scrollbar-width: thin; /* firefox */
    scrollbar-color: var(--thumbBgdColor) var(--scrollbarBgdColor);
}
::-webkit-scrollbar {
    width: var(--scrollbarSize);
    height: var(--scrollbarSize)
}
::-webkit-scrollbar-track {
    background: var(--scrollbarBgdColor);
}
::-webkit-scrollbar-thumb {
    background-color: var(--thumbBgdColor);
    border-radius: 8px;
    border: 3px solid var(--scrollbarBgdColor);
}
::-webkit-scrollbar-corner {
    background: rgba(0,0,0,0);
}
input {
    color: #ccc;
    background-color: #222;
    border-width: thin;
}
h3,h4 {
    font-size: 1.5em;
    margin-block-start: 10px;
    margin-block-end: 10px;
    color: var(--headingColor);
}
h3 > a {
    font-size: .7em;
    color: var(--linkColor);
    font-weight: var(--linkWeight); 
}
td,th {
    text-align: left;
    font-weight: normal;
}
tr {
    border: 2px solid black;
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
.Group div,
.Group > table {
    margin-bottom: 5px;
    padding-left: 5px;
}
.Group table {
    width: 100%;
}
col.c1, col.c2 {
    width: 3em;
}
.Title {
    font-size: 1.15em;
    color: var(--titleColor); 
}
.Label {
    display: inline-block;
    width: 8em;
    padding-left: 5px;
    padding-right: 10px;
}
#voicelist {
    overflow-y: auto;
    height: 11em;
    background-color: var(--voicelistBgd);
}
#voicelist tr:hover {
    background-color: var(--hoverBgd); 
}
#voicelist tr.active:hover,
#voicelist tr.active {
    background-color: var(--activeBgd);
}
#voicelist td {
    cursor: pointer;
}
:focus {
    outline: 1px dashed var(--titleColor);
}
</style>
<body >
    <H3>FluidSynth.clap  
        <a href="https://github.com/cannerycoders/fluidsynth.clap" target="_blank">github</a> |
        <a href="http://fluidsynth.org" target="_blank">fluidsynth.org</a>
    </H3>
    <div class="Group">
        <!-- just want a file-pathname widget, no drop-zone etc -->
        <div class="Title">Sound Font</div>
        <div><input id="filepath" value="default.sf2"></div>
    </div>
    <div class="Group">
        <div class="Title">Parameters</div>
        <div><div class="Label">Gain</div><input id="gain" type="number" value="1" min="0" max="8" step=".1"></div>
        <div><div class="Label">Voice, Bank, Prog</div><span id="voicename"></span> <span id="bank">0</span>, <span id="prog">0</span></div>
        <div class="Title">Voices</div>
        <table>
            <colgroup><col class="c1"><col class="c2"></colgroup>
            <tr><th>Bank</th><th>Prog</th><th>Name</th></tr>
        </table>
        <div id="voicelist"></div>
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
    let bank = document.querySelector("#bank");
    let prog = document.querySelector("#prog");
    let voicename = document.querySelector("#voicename");
    let tabEl = document.querySelector("#voicelist");
    try
    {
        let o = JSON.parse(json);
        let html = [];
        html.push("<table>");
        html.push("<colgroup><col class='c1'><col class='c2'></colgroup>");
        let i = 0;
        for(let p of o) // an array of obj, last is {}
        {
            if(!p.nm) break;
            html.push(`<tr tabindex='0' id='r${i++}'><td>${p.b}</td><td>${p.p}</td><td>${p.nm}</td></tr>\n`);
        }
        html.push("</table>");
        tabEl.innerHTML = html.join("");

        function changeVoice(evt)
        {
            let i = parseInt(evt.currentTarget.id.slice(1));
            let oldActive = tabEl.querySelector(".active");
            if(oldActive) oldActive.classList.toggle("active");
            evt.currentTarget.classList.toggle("active");

            let oi = o[i];
            prog.innerText = oi.p;
            bank.innerText = oi.b;
            voicename.innerText = oi.nm;
            tellHost("setparam", "prog0", `${oi.p}`); // value must be string
            tellHost("setparam", "bank0", `${oi.b}`); 
        }
        for(let row of tabEl.querySelectorAll("tr"))
        {
            row.onfocus = (evt) =>
            {
                changeVoice(evt);
            }
            row.onclick = (evt) =>
            {
                evt.currentTarget.focus();
                // changeVoice(evt);
            };
        }
    }
    catch(err)
    {
        el.innerHTML = `<span class='ERROR'>${err}<br>${json}</span>`;
    }
}
document.addEventListener("DOMContentLoaded", initBindings);
// we rely on focus and tab, shift-tab to select voices
// document.onkeydown = (evt) => { tellHost("log", `keycode: ${evt.keyCode}`); };
tellHost("dbg", "script loaded");
</script>

</body>
</html>
)";

bool
FluidsynthPlugin::guiCreate(const char *api, bool isFloating) noexcept 
{
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
    #if AS_TRANSIENT
    {
        m_window = new choc::ui::DesktopWindow({100, 100, 512, 510});
        // since we're installing a webview (both choc-aware), we needn't
        // trigger the creation of default content.
        m_window->windowClosed = [this]
        {
            _host.log(CLAP_LOG_INFO, "window-closed");
        };
        m_window->setContent(m_webview->getViewHandle());
    }
    #endif
    return true;
}

void
FluidsynthPlugin::updateVoices()
{
    if(!m_webview) return; // not in gui mode

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
            snprintf(buf, sizeof(buf), "fluidsynth.getrez unhandled %s", path.c_str());
            _host.log(CLAP_LOG_INFO, buf);
        }
        return std::nullopt;
    }
}
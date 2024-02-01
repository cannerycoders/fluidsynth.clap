/**
 * FluidInstance is associated within a single instance of
 * a FluidSynth.clap plugin.  We reside within and are
 * managed by the FluidGUI.
 */
class FluidInstance
{
    constructor(iid, sid, parentDiv, fluidGui)
    {
        this.fluidGui = fluidGui;
        this.iid = iid;
        this.sid = sid;
        parentDiv.insertAdjacentHTML("beforeend", `
<div class="Panel" id="panel${iid}">
  <h5>Settings for instance ${sid}</h5>
  <div class="Group">
    <!-- just want a file-pathname widget, no drop-zone etc -->
    <div class="Title">Sound Font</div>
    <div><input id="filepath" value="default.sf2"></div>
  </div>
  <div class="Group">
    <div class="Title">Parameters</div>
    <div id="params"></div>
    <div><div class="Label">Voice, Bank, Prog</div><span id="voicename"></span> <span id="bank">0</span>, <span id="prog">0</span></div>
    <div class="Title">Voices</div>
    <table>
        <colgroup><col class="c1"><col class="c2"></colgroup>
        <tr><th>Bank</th><th>Prog</th><th>Name</th></tr>
    </table>
    <div id="voicelist"></div>
  </div>
</div>`);
        this.panel = document.querySelector(`#panel${iid}`);
        this.paramsEl = this.panel.querySelector("#params");
        this.initBindings();
        this.fluidGui.getState(this.iid, this.sid);
    }

    handleEvent(evt)
    {
        switch(evt.data.msg)
        {
        case "show":
            this.panel.style.display = "block";
            break;
        case "hide":
            this.panel.style.display = "none";
            break;
        case "pluginSetState":
            this.updateState(evt.data.state);
            break;
        }
    }

    initBindings()
    {
        let sfel = this.panel.querySelector("#filepath");
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
                        this.fluidGui.setParam(this.iid, sfel.id, s);
                    });
                }
                else
                if(item.kind === "file")
                {
                    const file = item.getAsFile();
                    sfel.value = file.name;
                    this.fluidGui.setParam(this.iid, sfel.id, file.name);
                }
                else
                    this.fluidGui.log(`can't drop ${item.kind}`);
            }
        });
        for(let el of this.panel.querySelectorAll("input"))
        {
            el.onchange = (evt) =>
            {
                this.fluidGui.setParam(this.iid, evt.target.id, evt.target.value);
            };
        }
    }

    updateState(json)
    {
        try
        {
            this.state = JSON.parse(json);
            this.panel.querySelector("#filepath").value = this.state.sf;
            if(this.state.voices)
                this.updateVoices(this.state.voices);
            if(this.state.params) // a dict keyed by name. contains id, param, range
            {
                let html = [];
                for(let name in this.state.params)
                {
                    let p = this.state.params[name];
                    html.push(`<div><div class="Label">${name}</div>`);
                    let [min,max,step] = p.range;
                    if(step == null) step = (max - min) / 100;
                    if(step == 1 && min == 0 && max == 1)
                    {
                        let val = p.value ? "checked" : "";
                        html.push(`<input id="${p.id}" type="checkbox" ${val} value="${p.value}">`);
                    }
                    else
                    {
                        html.push(`<input id="${p.id}" type="number" value="${p.value}"`);
                        html.push(` min="${min}" max="${max}" step="${step}">`);
                    }
                    html.push("</div>");
                }
                this.paramsEl.innerHTML = html.join("");

                for(let el of this.paramsEl.querySelectorAll("input"))
                {
                    el.onchange = (evt) =>
                    {
                        if(evt.target.type == "checkbox")
                            this.fluidGui.setParam(this.iid, evt.target.id, evt.target.checked ? 1 : 0);
                        else
                            this.fluidGui.setParam(this.iid, evt.target.id, evt.target.value);
                    };
                }
            }
        }
        catch(err)
        {
            this.fluidGui.log("WARNING json botch " + err);
        }
    }

    updateVoices(voicelist)
    {
        this.voicelist = voicelist;
        let tabEl = this.panel.querySelector("#voicelist");
        let html = [];
        html.push("<table>");
        html.push("<colgroup><col class='c1'><col class='c2'></colgroup>");
        let i = 0;
        for(let p of this.voicelist) // an array of obj, last is {}
        {
            if(!p.nm) break;
            html.push(`<tr tabindex='0' id='r${i++}'><td>${p.b}</td><td>${p.p}</td><td>${p.nm}</td></tr>\n`);
        }
        html.push("</table>");
        tabEl.innerHTML = html.join("");

        for(let row of tabEl.querySelectorAll("tr"))
        {
            row.onfocus = (evt) =>
            {
                this.changeVoice(evt);
            }
            row.onclick = (evt) =>
            {
                evt.currentTarget.focus();
                // changeVoice(evt);
            };
        }
    }

    changeVoice(evt)
    {
        let bank = this.panel.querySelector("#bank");
        let prog = this.panel.querySelector("#prog");
        let voicename = this.panel.querySelector("#voicename");
        let tabEl = this.panel.querySelector("#voicelist");

        let i = parseInt(evt.currentTarget.id.slice(1));
        let oldActive = tabEl.querySelector(".active");
        if(oldActive) oldActive.classList.toggle("active");
        evt.currentTarget.classList.toggle("active");

        let oi = this.voicelist[i];
        prog.innerText = oi.p;
        bank.innerText = oi.b;
        voicename.innerText = oi.nm;
        this.fluidGui.setParam(this.iid, this.state.prog0.id, `${oi.p}`); // value must be string
        this.fluidGui.setParam(this.iid, this.state.bank0.id, `${oi.b}`); 
    }
}

class FluidGUI
{
    constructor()
    {
        this.panelsDiv = document.querySelector(".Panels"); 
        this.panelsDiv.innerHTML = `
    <div style="width:100%"><h3>FluidSynth.clap  
        <a href="https://github.com/cannerycoders/fluidsynth.clap" target="_blank">github</a> |
        <a href="http://fluidsynth.org" target="_blank">fluidsynth.org</a>
    </h3></div>`;
        this.instances = {};
        window.addEventListener("message", (evt) =>
        {
            if(evt.data.iid != null)
            {
                let sid = evt.data.sid;
                let iid = evt.data.iid;
                let inst = this.instances[sid];
                if(!inst)
                {
                    inst = new FluidInstance(iid, sid, this.panelsDiv, this);
                    this.instances[sid] = inst;
                }
                else
                if(inst.iid != iid)
                {
                    console.log("fluidsynth iid botch");
                }
                inst.handleEvent(evt);
            }
            else
                this.log("ERROR: unhandled message " + JSON.stringify(evt.data));
        });
    }

    setParam(iid, pid, value)
    {
        let anyOrigin = "*";
        parent.postMessage({
            msg: "pluginSetParam", 
            iid, pid, 
            val: value
        }, anyOrigin);
    }

    getState(iid, sid)
    {
        let anyOrigin = "*";
        // console.log("-------------------getState for " + iid);
        parent.postMessage({
            msg: "pluginGetState", 
            iid, sid
        }, anyOrigin);
    }

    log(msg)
    {
        let anyOrigin = "*";
        parent.postMessage({
            msg: "log", 
            val: msg
        }, anyOrigin);
    }
}

window.fluidGUI = new FluidGUI();
window.fluidGUI.log("Fluidsynth.clap GUI loaded");
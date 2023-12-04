class FluidInstance
{
    constructor(iid, parentDiv, ctx)
    {
        this.ctx = ctx;
        this.iid = iid;
        parentDiv.insertAdjacentHTML("beforeend", `
<div class="Panel" id="panel${iid}">
  <h5>Settings for instance ${iid}</h5>
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
</div>`);
        this.panel = document.querySelector(`#panel${iid}`);
        this.initBindings();
        this.ctx.requestState(this.iid);
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
        case "setPluginState":
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
                        this.ctx.setParam(this.iid, sfel.id, s);
                    });
                }
                else
                if(item.kind === "file")
                {
                    const file = item.getAsFile();
                    sfel.value = file.name;
                    this.ctx.setParam(this.iid, sfel.id, file.name);
                }
                else
                    this.ctx.log(`can't drop ${item.kind}`);
            }
        });
        for(let el of this.panel.querySelectorAll("input"))
        {
            el.onchange = (evt) =>
            {
                this.ctx.setParam(this.iid, evt.target.id, evt.target.value);
            };
        }
    }

    updateState(json)
    {
        try
        {
            let o = JSON.parse(json);
            if(o.voices)
                this.updateVoices(o.voices);
        }
        catch(err)
        {
            this.ctx.log("WARNING json botch " + err);
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
        this.ctx.setParam(this.iid, "prog0", `${oi.p}`); // value must be string
        this.ctx.setParam(this.iid, "bank0", `${oi.b}`); 
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
                let iid = evt.data.iid;
                let inst = this.instances[iid];
                if(!inst)
                {
                    inst = new FluidInstance(iid, this.panelsDiv, this);
                    this.instances[iid] = inst;
                }
                inst.handleEvent(evt);
            }
            else
                this.log("ERROR: unhandled message " + JSON.stringify(evt.data));
        });
    }

    setParam(iid, name, value)
    {
        parent.postMessage({msg: "log", val: `${iid}.setParam ${name} ${value}`}, "*");
    }

    requestState(iid)
    {
        // console.log("-------------------requestState for " + iid);
        parent.postMessage({msg: "getPluginState", iid: iid}, "*");
    }

    log(msg)
    {
        parent.postMessage({msg: "log", val: msg}, "*");
    }
}

window.fluidGUI = new FluidGUI();
window.fluidGUI.log("Fluidsynth.clap GUI loaded");
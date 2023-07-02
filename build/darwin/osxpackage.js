//
// hdiutil create -volname MyPlugin -srcfolder input.clap MyPlugin.dmg
const path = require("path");
const fs = require("fs");
const appdmg = require("appdmg");
if(process.argv.length != 6)
{
    console.error("usage: node osxpackage.js pluginName inputDir installHelp.rtf dmgPath");
    process.exit(-1);
}
let [_0, _1, pluginName, inputDir, installHelp, dmgPath] = process.argv;

let cfg = {
    target: dmgPath,
    basepath: ".",
    specification: {
        "title": `${pluginName} Installer`,
        "icon": path.join(inputDir, "Contents/Resources/clap.icns"),
        "background": "build/darwin/installbgd.png",
        "contents": [
            // grid-locations are quantized
            { "x": 415, "y": 375, "type": "link", "path": "/Library/Audio/Plug-Ins/CLAP" },
            { "x": 20, "y": 375, "type": "file", "path": installHelp },
            { "x": 185, "y": 375, "type": "file", "path": inputDir },
        ]
    },
};

try
{
    fs.unlinkSync(dmgPath);
}
catch(err)
{
    if(err.code != "ENOENT")
        console.log(JSON.stringify(err, null, 2));
}

// console.log("cfg:\n" + JSON.stringify(cfg, null, 2));
// process.exit(0);

const ee = appdmg(cfg);
ee.on("finish", function() {
    console.log("done...");
});

ee.on("error", function(err) {
    console.error("ERROR " + err);
});
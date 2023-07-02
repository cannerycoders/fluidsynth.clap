//
// hdiutil create -volname MyPlugin -srcfolder input.clap MyPlugin.dmg
const path = require("path");
const appdmg = require("appdmg");
if(process.argv.length != 5)
{
    console.error("usage: node osxpackage.js pluginName inputDir dmgPath");
    process.exit(-1);
}
let [_0, _1, pluginName, inputDir, dmgPath] = process.argv;

let cfg = {
    basepath: process.cwd(),
    target: dmgPath,
    specification: {
        "title": `${pluginName} Installer`,
        "icon": path.join(inputDir, "Contents/Resources/clap.icns"),
        "background": "build/darwin/background.png",
        "contents": [
            { "x": 448, "y": 344, "type": "link", "path": "/Library/Audio/Plug-Ins/CLAP" },
            { "x": 192, "y": 344, "type": "file", "path": inputDir }
        ]
    },
};

// console.log("cfg:\n" + JSON.stringify(cfg, null, 2));
// process.exit(0);

const ee = appdmg(cfg);
ee.on("finish", function() {
    console.log("done...");
});

ee.on("error", function(err) {
    console.error("ERROR " + err);
});
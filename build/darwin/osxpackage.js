//
// hdiutil create -volname MyPlugin -srcfolder input.clap MyPlugin.dmg

let execFileSync = require("child_process").execFileSync;
let devId = process.env.APPLEID_APPLICATION;
if(process.argv.length != 5)
{
    console.error("usage: node osxpackage.js pluginName inputDir dmgPath");
    process.exit(-1);
}
let [_0, _1, pluginName, inputDir, dmgPath] = process.argv;

let arglist = [
    "create", "-volname", pluginName, "-srcfolder", inputDir, dmgPath
];

try
{
    let stdout = execFileSync("hdiutil", arglist).toString().trim();
    if(stdout.length)
        console.log(`INFO hdiutil:\n${stdout}`);
}
catch(err)
{
    console.error(`ERROR hdiutil:\n${err}`);
}
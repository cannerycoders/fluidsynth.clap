//
// https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/
//
// xcrun notarytool submit myplugin.zip --apple-id you@email.com --password myPassword --team-id ABC123 --wait

let fs = require("fs"); // require("fs/promises");
let path = require("path");
let execFileSync = require("child_process").execFileSync;
let devId = process.env.APPLEID_APPLICATION;
if(!devId)
{
    console.error("APPLEID_APPLICATION not found in environment.")
    process.exit(-1);
}
if(process.argv.length != 3)
{
    console.error("usage: node osxsign.js path_to_pkg_root");
    process.exit(-1);
}
let projRoot = process.argv[2];
let arglist = [
    "--force", "-s", devId, "--deep", "--strict", "--timestamp",
    projRoot,
];

try
{
    let stdout = execFileSync("codesign", arglist).toString().trim();
    if(stdout.length)
        console.log(`INFO codesign:\n${stdout}`);
}
catch(err)
{
    console.error(`ERROR codesign:\n${err}`);
}
//
// this script is run on macos by jsmk package
// 
// https://developer.apple.com/forums/thread/130855
// just iterate over all the files in the package
//  -- only sign binaries
//  -- only apply entitlements to apps
// https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/
//
// codesign --sign magic --force --timestamp \
//    --options runtime 
//    --entitlements $thisdir/entitlements.plist 
//    file

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
    "codesign", "--force", "-s", devId, "--deep", "--strict", "--options=runtime", "--timestamp",
    "--entitlements", "build/darwin/entitlements.plist",
    projRoot,
];

try
{
    console.log(`codesign -s XXX ${arglist.slice(4).join(" ")}`);
    let stdout = execFileSync(arglist[0], arglist.slice(1)).toString().trim();
    if(stdout.length)
        console.log(`INFO codesign:\n${stdout}`);
}
catch(err)
{
    console.error(`ERROR codesign:\n${err}`);
}
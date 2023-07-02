//
// https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/
//
// xcrun notarytool submit myplugin.zip --apple-id you@email.com --password myPassword --team-id ABC123 --wait

let fs = require("fs"); // require("fs/promises");
let path = require("path");
let execFileSync = require("child_process").execFileSync;
let appleId = process.env.APPLEID;
let teamId = process.env.APPLEID_TEAM;
let accessToken = process.env.APPLEID_ACCESS_TOKEN;
if(!appleId || !teamId || !accessToken)
{
    console.error("APPLEID, APPLEID_TEAM, APPLEID_ACCESS_TOKEN required in environment.")
    process.exit(-1);
}
if(process.argv.length != 3)
{
    console.error("usage: node osxnotarize.js myplugin.dmg");
    process.exit(-1);
}
let notaryPkg = process.argv[2];
let argv = [
    "xcrun", "notarytool", "submit", notaryPkg, 
      "--apple-id", appleId,
      "--password", accessToken,
      "--team-id", teamId, 
      "--wait",
];

try
{
    let stdout = execFileSync(argv[0], argv.slice(1)).toString().trim();
    if(stdout.length)
        console.log(`INFO osxnotarize:\n${stdout}`);
    
    argv = ["xcrun", "stapler", "staple", notaryPkg]
    stdout = execFileSync(argv[0], argv.slice(1)).toString().trim();
    if(stdout.length)
        console.log(`INFO osxnotarize/stapler:\n${stdout}`);
}
catch(err)
{
    console.error(`ERROR osxnotarize:\n${err}`);
}
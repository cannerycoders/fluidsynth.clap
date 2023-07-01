//
// https://developer.apple.com/forums/thread/130855
// just iterate over all the files in the package
//  -- only sign binaries
//  -- only apply entitlements to apps
//
// codesign --sign magic --force --timestamp \
//    --options runtime 
//    --entitlements ../build/entitlements.plist 
//    file

let fs = require("fs"); // require("fs/promises");
let path = require("path");
let execFileSync = require("child_process").execFileSync;
        
let stdout = execFileSync("codesign", arglist).toString().trim();
if(stdout.length)
    console.log(`codesign ${arglist}:\n${stdout}`);
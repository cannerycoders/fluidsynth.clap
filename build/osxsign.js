// here we sign the app, more notes in __Root.jsmk.
// currently we're launched from osxFixup.sh

// https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/

const path = require("path");
const buildname = path.parse(process.argv[1]).name; // ie build4electron
process.env.DEBUG=`electron-osx-sign ${buildname}`;
// process.env.DEBUG=`electron-packager electron-osx-sign electron-notarize ${buildname}`;

const { execSync } = require('child_process');
const pkg = require("./package.json"); // for version
const d = require("debug")(buildname);

const appDir = process.argv[2] || path.join(process.env.FIDDLEINST, "Fiddle.app");

// https://kilianvalkhof.com/2019/electron/notarizing-your-electron-application/
async function DoitOSX()
{
    // let appDir =  "../_install/clang-arm64-darwin-vanilla-debug/Fiddle-0.4.0/Fiddle-darwin-arm64/Fiddle.app";
    let appPath = path.join(__dirname, appDir);
    const {signAsync} = require("electron-osx-sign");
    d("code-signing");
    await signAsync({
        app: `${appDir}`,
        platform: "darwin", //  not "mas" (not through the app Store)
        type: "distribution",
        entitlements: "../build/entitlements.plist",
        "entitlements-inherit": "../build/entitlements.plist",
        "gatekeeper-assess": false, 
        hardenedRuntime: true,
    });
}

async function Doit()
{
    await DoitOSX()
}

Doit()

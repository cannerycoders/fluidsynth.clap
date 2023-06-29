// Currently our job is to populate fixup packaged dlls so they they refer
// to one another via relative paths.   Moreover, on MacOS, security requires
// that they be located in Frameworks and signed.
//
// usage `node fixupDLL.js appInstallDir`
//
let fs = require("fs"); // require("fs/promises");
let path = require("path");
let execFileSync = require("child_process").execFileSync;

console.log("" + process.argv);
process.exit(0);

if(process.argv.length != 3)
    throw new Error(process.argv[1] + " requires 3 args, got " + process.argv.length);

let appDir = process.argv[2];
let chugloc, chugreldir, dlldir;
if(process.platform == "darwin")
{
    dlldir = `${appDir}/Contents/Frameworks`; 
    plugdir = 
    chugloc = `${appDir}/Contents/Resources/chuck/chugins`;
    chugreldir = "../../../Frameworks";
}
else
{
    dlldir = appDir;
    chugloc = `${appDir}/resources/chug/chugins`;
    chugreldir = "../../..";
}

console.log(`\n\nfixupDLLs dlldir:${dlldir}----\n\n`);

let filesToFix = 
{
    darwin:
    [
        [`${chugloc}/FluidSynth.chug`, chugreldir]
    ],
    win32: 
    [
        // so far fluid-support happen in _Proj.jsmk since
        // no fixups are required.
    ]
}[process.platform];


if(process.platform == "darwin")
{
    // https://gitlab.kitware.com/cmake/cmake/-/issues/21854
    // during linking -Wl,-no_adhoc_codesign

    let doneFindDep = {};
    let depsMap = {}; // maps libname to list of deplibnames

    // first perform the copy
    for(let [file, rel] of filesToFix) // ie: the chugins with external deps
    {
        osxFindDependencies(file, doneFindDep, depsMap);
        delete doneFindDep[file]; // don't want to copy/move these file
        osxFixupRefs(file, null, `@loader_path/${rel}`, depsMap);
    }

    let doneCopy = {};
    for(let key in doneFindDep)
        osxCopyOne(key, doneCopy);
    
    // now fixup all refererences with install_name_tool0
    // console.log("depsMap\n" + JSON.stringify(depsMap, null, 2));
    for(let key in doneCopy)
        osxFixupRefs(key, doneCopy[key], "@loader_path", depsMap);

    function osxFixupRefs(tgtfile, oldid, prefix, depsMap)
    {
        // 
        let filebase = path.basename(tgtfile);
        console.log(`Fixup ${filebase} id(${oldid})`);
        let deps = depsMap[filebase] || [];
        let arglist = ["-id", filebase];
        for(let ref of deps)
        {
            let basename = path.basename(ref);
            let newref = `${prefix}/${basename}`;
            arglist.push(...["-change", ref, newref]);
            console.log(`-change ${ref}  to ${newref}}`);
        }
        arglist.push(tgtfile);
        let stdout = execFileSync("install_name_tool", arglist).toString().trim();
        if(stdout.length)
            console.log(`install_name_tool ${arglist}:\n${stdout}`);
    }
    
    // reentrant/recursive routine
    function osxFindDependencies(file, done, deps)
    {
        if(done[file])
            return;

        let stdout = execFileSync("/usr/bin/otool", ["-L", file]).toString();
        let lines = stdout.trim().split("\n").filter((l) => l.length);
        // line 0 is the file
        // line 1 is the original name of the file
        // 
        // console.log(`doing ${path.basename(file)} -----`)
        done[file] = true;
        let tgtfile = path.basename(file);
        deps[tgtfile] = []; // used during postprocessing for install_name_tool
        for(let i=2; i<lines.length;i++)
        {
            let l = lines[i].trim();
            if(l.startsWith("/usr/lib") || l.startsWith("/System"))
                continue;
            else
            {
                let j = l.indexOf(".dylib")
                let fn = l.substr(0, j+6);
                deps[tgtfile].push(fn);
                if(!done[fn])
                {
                    // console.log(fn);
                    osxFindDependencies(fn, done, deps);
                }
            }
        }
    }

    // Copy src to dlldir/dstfile, done is used to ensure we don't
    // recur indefinitely. done's key is the dst filename and this is
    // used when the "same" .dll may exist in multiple locations.
    // First one wins. In the common case where src is a symlink
    // we deference the link (eg: libc.3.so => libc.3.0.0.so).
    // This was currently preferred so as to not produce the hub-bub
    // of links in our distribution.
    function osxCopyOne(src, done)
    {
        let dst = path.join(dlldir, path.basename(src))
        let stat = fs.lstatSync(src);
        if(stat.isSymbolicLink())
        {
            let link = fs.readlinkSync(src);
            if(!path.isAbsolute(link))
            {
                // XXX: cbb
                link = path.join(path.dirname(src), fs.readlinkSync(src)); 
            }
            src = link; 
        }
        if(!done[dst])
        {
            console.log(`copying ${path.basename(src)} to dlldir`); 
            fs.copyFileSync(src, dst);
            fs.chmodSync(dst, 0o755);
            done[dst] = src;
            // remove the code-signing before we can invoke install_name_tool
            console.log("remove signature...");
            let stdout = execFileSync("/usr/bin/codesign", 
                ["--remove-signature", dst]).toString().trim();
            if(stdout.length)
                console.log(`codesign --remove-signature ${dst}\n . ${stdout}`);

            /*
            console.log("bitcode_strip");
            stdout = execFileSync("xcrun", 
                ["bitcode_strip", dst, "-r", "-o", dst]).toString().trim();
            if(stdout.length)
                console.log(`bitcode_strip ${dst}\n . ${stdout}`);
            */
        }
        else
            console.log(`skipping ${src} in favor of ${done[dst]}`);
    }
}

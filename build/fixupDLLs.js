// Currently our job is to populate fixup packaged dlls so they they refer
// to one another via relative paths.   On MacOS, security policies require
// that dylibs be located in Frameworks and signed.
//
// https://stackoverflow.com/questions/49223687/my-target-is-dynamically-linked-against-libraries-from-brew-how-to-bundle-for-d
//
// usage `node fixupDLL.js targetDir  _install/.../plugin1.clap ...`
//
let fs = require("fs"); // require("fs/promises");
let path = require("path");
let execFileSync = require("child_process").execFileSync;

if(process.argv.length < 4)
    throw new Error(process.argv[1] + " requires 4+ args, got " + process.argv.length);

let targetDir = process.argv[2];
let filesToFix = process.argv.slice(3);

console.log(`\n\nfixupDLLs targetDir:${targetDir}, fixing: ${filesToFix}----\n\n`);
fs.mkdirSync(targetDir, { recursive: true });

if(process.platform == "darwin")
{
    // https://gitlab.kitware.com/cmake/cmake/-/issues/21854
    // during linking -Wl,-no_adhoc_codesign

    let doneFindDep = {};
    let depsMap = {}; // maps libname to list of deplibnames

    // first perform the copy
    let rel = "../Frameworks"; // relative path from file to its dylibs.
    for(let file of filesToFix) // ie: the chugins with external deps
    {
        osxFindDependencies(file, doneFindDep, depsMap);
        delete doneFindDep[file]; // don't want to copy/move these files, they're already in place
        osxFixupRefs(file, null, `@loader_path/${rel}`, depsMap);
    }

    console.log("\n\n---- copying -----\n\n\n");

    // fixup up refs of incoming files, transitive dependents happen after the ensuing copy.
    let doneCopy = {};
    for(let key in doneFindDep)
    {
        console.log(key);
        osxCopyOne(key, doneCopy);
    }

    console.log("\n\n---- fixing -----\n\n\n");

    // now fixup all refererences with install_name_tool
    // console.log("depsMap\n" + JSON.stringify(depsMap, null, 2));
    for(let key in doneCopy)
    {
        // nb: key is the copied fileref, doneCopy[key] is the orig
        osxFixupRefs(key, doneCopy[key], `@loader_path/${rel}`, depsMap);
    }

    // depsMap 
    //   key is local dll name (eg libvorbis.0.dylib), 
    //   value  is [original deps eg:/opt/homebrew/opt/opus/lib/libopus.0.dylib]
    function osxFixupRefs(tgtfile, oldid, prefix, depsMap)
    {
        let filebase = path.basename(tgtfile); // to lookup in depsMap
        console.log(`Fixup ${filebase} (original:${oldid})`);
        let deps = depsMap[filebase] || [];
        let arglist = ["-id", filebase];
        for(let ref of deps)
        {
            let basename = path.basename(ref);
            let newref = `${prefix}/${basename}`;
            arglist.push("-change", ref, newref);
        }
        arglist.push(tgtfile);

        console.log(`fixing ${filebase}\n\t${arglist}`);
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
        let dst = path.join(targetDir, path.basename(src))
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
            console.log(`copying ${path.basename(src)} to targetDir`); 
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

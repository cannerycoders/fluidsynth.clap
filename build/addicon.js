let arglist = process.argv.slice(2); // [node, scriptname, args]
let inputs = {};
for(let i=0;i<arglist.length;)
{
    let a = arglist[i++];
    if(a.startsWith("--"))
    {
        let fields = a.split("="); // booleans may not have a value
        let tok = fields[0].slice(2); // skip --
        let val = fields[1] || true; 
        inputs[tok] = val;
    }
    else
        throw new Error("Invalid/unknown arg:" + a);
}

console.log(arglist[1], JSON.stringify(inputs, null, 2));
switch(process.platform)
{
case "win32":
    {
        let rcedit = require("rcedit");
        let options =
        {
            "version-string": {
                CompanyName: "Cannery Coders",
                FileDescription: "onda app",
                LegalCopyright: "copyright 2023 Cannery Coders, all right reserved",
                ProductName: "onda"
            },
            "file-version":  "0.1.0",  // - File's version to change to.
            "product-version": "0.1.0", // Product's version to change to.
            icon: inputs["--icon"],   // Path to the icon file (.ico) to set as the exePath's default icon.
            "requested-execution-level":  "asInvoker", // from: asInvoker, highestAvailable, or requireAdministrator. See here for more details.
            "application-manifest": "" // String path to a local manifest file to use. 
        };
        let exepath = inputs["--app"];
        rcedit(exepath, options)
        .then((x, y) =>
        {
            console.log(x, y);
        });
        break;
    }
default:
    throw new Error("yo");
}


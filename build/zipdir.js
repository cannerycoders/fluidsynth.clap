// usage:  node zipdir.js dirtozip  outputfilename (.zip)
const path = require("path")
const zip = require("zip-a-folder").zip;

const inputdir = process.argv[2];
const outputfile = process.argv[3];

class TestMe 
{
    static async main() {
        await zip(inputdir, outputfile);
        console.log(`wrote ${outputfile}`);
    }
}

TestMe.main();
const fs = require('fs');
const path = require('path');
const main = process.argv[2];
const out = 'out/submission.c';

let source = fs.readFileSync(path.join(__dirname, main), 'utf8');
let regex = /#include "(.*)"/g;

console.log("Replacing the following includes:");
let match;
while(match = regex.exec(source)){
    let line = match[0];
    let include = match[1];
    console.log(" * " + include);
    let content = fs.readFileSync(path.join(__dirname, include), 'utf8');
    source = source.replace(line, content);
}

if(!fs.existsSync(path.dirname(out)))
    fs.mkdirSync(path.dirname(out), { recursive: true });

fs.writeFileSync(out, source);

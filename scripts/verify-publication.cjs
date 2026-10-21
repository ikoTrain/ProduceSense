const fs=require('fs'),p=require('path'),assert=require('node:assert/strict');
const root=p.resolve(__dirname,'..');
function walk(d){return fs.readdirSync(d,{withFileTypes:true}).flatMap(e=>e.name==='.git'?[]:e.isDirectory()?walk(p.join(d,e.name)):[p.join(d,e.name)]);}
const list=walk(root);
for(const f of list){assert.ok(fs.statSync(f).size<100*1024*1024,'GitHub file size: '+f);if(/\.(md|html|svg|cjs|ps1|ino|h|json|txt|csv)$/.test(f)){const s=fs.readFileSync(f,'utf8');assert.ok(!/gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,}|-----BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY-----/.test(s),'Credential-like content: '+f);}}
for(const f of ['README.md','docs/article.md','data/README.md','firmware/ProduceClassifier/README.md']){
 const s=fs.readFileSync(p.join(root,f),'utf8');assert.ok(!/[А-Яа-яІіЇїЄє]/u.test(s),'Publication text is not fully English: '+f);
 for(const m of s.matchAll(/!?\[[^\]]*\]\(([^)]+)\)/g)){if(!/^(https?:|#)/.test(m[1]))assert.ok(fs.existsSync(p.resolve(root,p.dirname(f),m[1])),'Broken link in '+f+': '+m[1]);}
}
console.log('Verified local document links, English prose, file sizes and credential-pattern scan.');

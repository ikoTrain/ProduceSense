const fs=require('fs'),p=require('path'),assert=require('node:assert/strict'),crypto=require('crypto');
const root=p.resolve(__dirname,'..'),read=f=>JSON.parse(fs.readFileSync(p.join(root,f),'utf8').replace(/^\uFEFF/,''));
const files=d=>fs.readdirSync(p.join(root,d)).filter(f=>f.endsWith('.json'));
const model=read('model/three-class-v1.json');
const mean=xs=>xs[0].map((_,j)=>xs.reduce((a,x)=>a+x[j],0)/xs.length);
const keys=['F, 1000','F, 5000','F, 20000','R, 1000','R, 5000','R, 20000'];
const raw=files('data/two-electrode').map(f=>({file:f,...read('data/two-electrode/'+f)})).filter(r=>model.training_ids.includes(r.specimen_id || (r.file.startsWith('P-unknown-')?'P_UNIDENTIFIED':'')));
const training=model.training_ids.map(id=>{const records=raw.filter(r=>(r.specimen_id||(r.file.startsWith('P-unknown-')?'P_UNIDENTIFIED':''))===id);assert.equal(records.length,2,'two placements per training specimen: '+id);return{id,label:records[0].class,x:mean(records.map(r=>keys.map(k=>{const q=r.summary.find(s=>s.group===k);assert.equal(q.count,30);assert.equal(q.rail_count,0);return q.mv_mean;})))};});
function fit(rs){const mu=mean(rs.map(r=>r.x)),sd=mu.map((m,j)=>Math.sqrt(rs.reduce((s,r)=>s+(r.x[j]-m)**2,0)/rs.length)||1);return{labels:model.labels,mu,sd,centers:model.labels.map(c=>mean(rs.filter(r=>r.label===c).map(r=>r.x.map((v,j)=>(v-mu[j])/sd[j]))))};}
function predict(m,x){const z=x.map((v,j)=>(v-m.mu[j])/m.sd[j]),d=m.centers.map(c=>c.reduce((s,v,j)=>s+(v-z[j])**2,0));return m.labels[d.indexOf(Math.min(...d))];}
const rebuilt=fit(training);for(const k of ['mu','sd','centers'])assert.ok(JSON.stringify(rebuilt[k])===JSON.stringify(model[k])||rebuilt[k].flat().every((v,i)=>Math.abs(v-model[k].flat()[i])<1e-9),'model reconstruction: '+k);
const records=files('data/final-series').map(f=>read('data/final-series/'+f));assert.equal(records.length,30);
const hash=crypto.createHash('sha256').update(fs.readFileSync(p.join(root,'model/three-class-v1.json'))).digest('hex').toUpperCase();
for(const r of records){assert.equal(r.model_file_sha256,hash);assert.equal(r.invalid,false);assert.equal(r.cycles,6);assert.equal(r.elapsed_ms,7600);assert.equal(predict(model,r.features_mv),r.predicted);assert.equal(r.correct,r.class===r.predicted);}
const specimens=[...new Set(records.map(r=>r.specimen_id))];assert.equal(specimens.length,15);let maxShift=0;
for(const id of specimens){const rr=records.filter(r=>r.specimen_id===id).sort((a,b)=>a.installation-b.installation);assert.equal(rr.length,2);assert.equal(rr[0].class,rr[1].class);maxShift=Math.max(maxShift,Math.abs(rr[1].features_mv[0]-rr[0].features_mv[0])/rr[0].features_mv[0]*100);}
const combinations=[];for(const a of training.filter(r=>r.label==='apple'))for(const t of training.filter(r=>r.label==='potato'))for(const c of training.filter(r=>r.label==='cucumber')){const rs=[a,t,c],m=fit(rs);combinations.push({training:3,ids:rs.map(r=>r.id),correct:records.filter(r=>predict(m,r.features_mv)===r.class).length});}
combinations.push({training:6,ids:training.map(r=>r.id),correct:records.filter(r=>predict(model,r.features_mv)===r.class).length});
const summary={model_sha256:hash,training_specimens:training.length,final_specimens:specimens.length,placements:records.length,correct:records.filter(r=>r.correct).length,elapsed_ms:7600,max_relative_F1_change_percent:maxShift,labels:model.labels,confusion:model.labels.map(a=>model.labels.map(b=>records.filter(r=>r.class===a&&r.predicted===b).length)),training_size:{note:'Post-hoc analysis of an already examined test set; not independent validation.',runs:combinations}};
fs.mkdirSync(p.join(root,'analysis'),{recursive:true});fs.writeFileSync(p.join(root,'analysis/results.json'),JSON.stringify(summary,null,2)+'\n');
fs.writeFileSync(p.join(root,'analysis/training-specimens.json'),JSON.stringify(training,null,2)+'\n');
const csv=['specimen_id,class,placement,F1_mV,F5_mV,F20_mV,R1_mV,R5_mV,R20_mV,predicted,correct,elapsed_ms',...records.sort((a,b)=>a.specimen_id.localeCompare(b.specimen_id)||a.installation-b.installation).map(r=>[r.specimen_id,r.class,r.installation,...r.features_mv,r.predicted,r.correct,r.elapsed_ms].join(','))].join('\n')+'\n';fs.writeFileSync(p.join(root,'analysis/final-series.csv'),csv);
console.log(JSON.stringify({training:training.length,specimens:specimens.length,placements:records.length,correct:summary.correct,model_hash_matches:true,max_F1_change_percent:maxShift,training_size_runs:combinations.length},null,2));

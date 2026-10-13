const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const input = process.argv[2] || 'out/crunchyroll/final';
const out = process.argv[3] || 'src/io';
const read = name => JSON.parse(fs.readFileSync(path.join(input,name+'.json'),'utf8'));
const entries=read('entries'), sourceGroups=read('groups');
const policy=JSON.parse(fs.readFileSync('scripts/crunchyroll-storylines.json','utf8'));
const mergeBySource=new Map();
for(const rule of policy.merges)for(const name of rule.sources){
  assert(!mergeBySource.has(name),'Duplicate storyline rule: '+name);
  assert(sourceGroups.some(g=>g.name===name),'Storyline rule source not found: '+name);
  mergeBySource.set(name,rule);
}
const grouped=new Map();
for(const g of sourceGroups){
  const rule=mergeBySource.get(g.name),name=rule?.name||g.name;
  if(!grouped.has(name))grouped.set(name,{name,rank:0,description:rule?.reason||'',members:[],source_series_ids:[]});
  grouped.get(name).members.push(...g.members);
  grouped.get(name).source_series_ids.push(g.source_series_id);
}
const groups=[...grouped.values()].sort((a,b)=>a.name.localeCompare(b.name)).map((g,id)=>({...g,id,members:g.members.sort((a,b)=>a-b)}));
for(const g of groups)for(const id of g.members){assert(entries[id]);entries[id].ip_group=g.id;}
const normalize=s=>s.normalize('NFKC').toLowerCase().replace(/[^a-z0-9]+/g,' ').trim();
const original=fs.readFileSync('src/io/animeDatabase.1.dat','utf8');
const rankings=[...original.matchAll(/^\{(\d+),(\d+),"([^"]*)"/gm)].map(m=>({id:Number(m[1]),rank:Number(m[2]),title:m[3]}));
assert(rankings.length>0,'No original ranking records parsed.');
const rankTransfers=[];
for(const e of entries)e.rank=0;
for(const old of rankings.filter(r=>r.rank>0)){
  const title=policy.rank_aliases[old.title]||old.title;
  const matches=entries.filter(e=>normalize(e.title)===normalize(title));
  assert.equal(matches.length,1,'Nonzero rank must have one exact/curated match: '+old.title);
  matches[0].rank=old.rank;
  rankTransfers.push({source_title:old.title,source_id:old.id,entry_id:matches[0].id,title:matches[0].title,rank:old.rank});
}
const substitutions=[];
function quote(text){
  assert(!/[\r\n]/.test(text));
  let opening=true;
  const normalized=text.replace(/"/g,()=>{const q=opening?'“':'”';opening=!opening;return q;});
  if(normalized!==text)substitutions.push({original:text,saved:normalized});
  return '"'+normalized+'"';
}
const vec=values=>'{'+values.join(',')+'}';
const lookups=fs.readFileSync('src/io/animeLookupTables.1.dat','utf8');
function lookup(name){
  const block=lookups.split(new RegExp('1\\|'+name+'\\|\\d+\\|1\\|0\\|'))[1]?.split(/\r?\n1\|/)[0];
  assert(block,'Missing lookup '+name);
  return [...block.matchAll(/\{"([^"\r\n]*)"\}/g)].map(m=>m[1]);
}
const lang={ja:'Japanese',en:'English',ko:'Korean',zh:'Chinese',es:'Spanish',fr:'French'};
const unmapped=new Set();
function languages(locales,table){const values=lookup(table);return [...new Set(locales.map(l=>{const id=values.indexOf(lang[l.split('-')[0]]);if(id<0)unmapped.add(l);return id;}).filter(id=>id>=0))].sort((a,b)=>a-b);}
const channel=lookup('channels').indexOf('Crunchyroll');assert(channel>=0);
function write(name,section,schema,rows){
  const output=`ADP0|1002|0|1|\n\n0|${section}|${rows.length}|${schema.length}|1|\n\n[${schema.join(',')}]\n\n${rows.map(r=>'{'+r.join(',')+'}').join('\n')}\n`;
  fs.writeFileSync(path.join(out,name),output,'utf8');
}
const entryRows=entries.map((e,i)=>{
  assert.equal(e.id,i);assert.equal(groups[e.ip_group]?.members.includes(e.id),true);
  assert(Number.isInteger(e.min_runtime)&&e.min_runtime>=0);
  return [e.id,e.rank,quote(e.title),quote(''),'','',vec([]),vec([channel]),vec(languages(e.source_audio_locales,'audios')),vec(languages(e.source_subtitle_locales,'subtitles')),'',e.episode_count,e.min_runtime,vec([]),e.ip_group];
});
const groupRows=groups.map((g,i)=>{
  assert.equal(g.id,i);const members=g.members.map(id=>{const e=entries[id];assert.equal(e.ip_group,g.id);return e;});
  // Required format-dependent counts remain zero until entry formats are known.
  // Group genres uses a legacy VECU8 schema tag but a VECU32 member. Leave
  // this unknown optional field missing instead of emitting a mistyped vector.
  return [g.id,g.rank,quote(g.name),quote(g.description),vec(g.members),'',members.reduce((n,e)=>n+e.min_runtime,0),0,0,0,'','','',0,1];
});
assert.equal(groups.reduce((n,g)=>n+g.members.length,0),entries.length);
fs.mkdirSync(out,{recursive:true});
const entrySchema=['id:3;0','rank:3;3','title:5;0','alt_title:5;0','format:1;2','status:1;2','genres:6;2','channels:6;2','audios:6;2','subtitles:6;2','fan_service:1;1','episode_count:2;1','min_runtime:3;1','characters:8;2','ip_group:3;2'];
const groupSchema=['id:3;0','rank:3;3','name:5;0','description:5;0','members:8;0','genres:6;2','knownRuntimeMinutes:4;0','episodeCount:3;0','movieCount:3;0','seriesCount:3;0','fanServiceLevel:1;0','status:1;0','runtimeComplete:4;0','compiledFromEntryRevision:4;0','compilationVersion:4;0'];
const sourceEntrySchema=original.match(/^\[(.*)\]/m)[1].split(',').map(f=>f.split(':')[0]);
assert.deepEqual(entrySchema.map(f=>f.split(':')[0]),sourceEntrySchema);
const sourceGroupSchema=fs.readFileSync('src/io/groupDatabase.1.dat','utf8').match(/^\[(.*)\]/m)[1].split(',');
assert.deepEqual(groupSchema,sourceGroupSchema);
write('animeDatabase.crunchyroll.dat','ANIME_DATA',entrySchema,entryRows);
write('groupDatabase.crunchyroll.dat','GROUP_DATA',groupSchema,groupRows);
fs.writeFileSync(path.join(input,'storyline-import.json'),JSON.stringify({entries,groups,rank_transfers:rankTransfers,merges:policy.merges},null,2));
fs.writeFileSync(path.join(out,'crunchyroll-import-notes.md'),`# Crunchyroll database pair\n\n${entries.length} qualifying entries and ${groups.length} storyline groups in ADP0 version 1002 format. Load these files together as a separate dataset. IDs start at zero and are local to this pair; do not append to the originals without remapping IDs. Original databases are unchanged.\n\nBoth files contain all 15 original fields, in the original order. Entry type tags use the current enum because the original entry header has obsolete tags. The group schema is identical to the original. Unknown optional scalars are blank, empty text is quoted, and empty vectors are {}. Requires the accompanying parser fix for blank non-text fields. Loaded unknown values still become C++ defaults; the structs do not retain missing-value flags. Required episode/movie/series group counts are zero placeholders until formats are classified; runtimes and membership are populated.\n\nGrouping policy: see ../../docs/storyline-groups.md. Explicit merges are reproducible in scripts/crunchyroll-storylines.json; default source-series groupings are provisional where not reviewed. SAO and Progressive form one group; Alternative GGO remains separate.\n\nRank transfers: ${rankTransfers.map(r=>r.source_title+' → '+r.title+' (rank '+r.rank+')').join('; ')||'none'}. Other entries and all groups are unranked. No row-order rankings were inferred.\n\nLocales without existing lookup values: ${[...unmapped].sort().join(', ')}. Source JSON retains them.\n\nThe parser has no escaped-quote support. Display quote substitutions:\n${substitutions.map(s=>'- '+s.original+' → '+s.saved).join('\n')}\n`);
console.log(`Wrote ${entries.length} entries and ${groups.length} groups to ${out}.`);

const assert = require('node:assert/strict');
const { match } = require('./match-crunchyroll-catalog.cjs');
const eps = (series='SERIES', season='S1', prefix='E') => Array.from({length:5},(_,i)=>({id:prefix+i,series_id:series,series_title:series,season_id:season,season_title:'Season 1',season_number:1,episode_number:i+1,title:'Episode '+i,duration_ms:1440000,versions:[]}));
const history = data => ({complete:true,next_page:null,error:null,data});
const h = (e,fully=true) => ({id:e.id,fully_watched:fully,panel:{title:e.title,episode_metadata:{...e}}});
const season = (data,id=data[0].season_id) => ({requested_season_id:id,complete:true,total:data.length,data});
const catalog = (...seasons) => ({complete:true,seasons});
const a=eps();
let r=match(history(a.slice(0,4).map(e=>h(e))),catalog(season(a)));
assert.equal(r.entries.length,1);assert.equal(r.seasons[0].watched_fraction,0.8);
r=match(history(a.slice(0,3).map(e=>h(e))),catalog(season(a)));
assert.equal(r.entries.length,0);
// Five language versions are still five episodes, not ten.
const dub=eps('SERIES','DUB','D');
for(let i=0;i<5;i++)dub[i].versions=[{guid:a[i].id}];
r=match(history(a.slice(0,4).map(e=>h(e))),catalog(season(a),season(dub)));
assert.equal(r.seasons.length,1);assert.equal(r.seasons[0].episode_count,5);assert.equal(r.seasons[0].fully_watched_episode_count,4);
// A wrong alias must never confer completion on unrelated content.
const wrong=eps('SERIES','OTHER','X');for(let i=0;i<5;i++){wrong[i].title='Different '+i;wrong[i].versions=[{guid:a[i].id}];}
r=match(history(a.map(e=>h(e))),catalog(season(wrong)));
assert.equal(r.entries.length,0);assert.equal(r.conflicts.length,5);
// Same ID/title/series remains reliable when a recap's numeric label changes.
const recap=h(a[0]);recap.panel.episode_metadata.episode_number=null;
r=match(history([recap,...a.slice(1).map(e=>h(e))]),catalog(season(a)));
assert.equal(r.seasons[0].fully_watched_episode_count,5);
// Partial plays do not count; missing panels need direct ID evidence.
r=match(history([h(a[0],false),{id:a[1].id,fully_watched:true}]),catalog(season(a)));
assert.equal(r.seasons[0].fully_watched_episode_count,1);assert.equal(r.summary.missing_metadata_recovered,1);
const black=eps('Black Clover');
r=match(history(black.map(e=>h(e))),catalog(season(black)));
assert.equal(r.seasons[0].decision,'excluded_user_unfinished');assert.equal(r.entries.length,0);
// Returned partial lists cannot supply a denominator.
r=match(history(a.map(e=>h(e))),catalog({...season(a),total:6}));
assert.equal(r.entries.length,0);assert.equal(r.invalid_catalogs.length,1);
assert.throws(()=>match({...history([]),complete:false},catalog()),/complete history/);
console.log('Passed: 80% boundary, language consolidation, bad aliases, recap numbering, partial playback, missing-panel recovery, explicit exclusions, incomplete catalogs.');

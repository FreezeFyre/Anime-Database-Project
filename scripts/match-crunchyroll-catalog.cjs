// node scripts/match-crunchyroll-catalog.cjs <history.json> <catalog.json> [output]
// Review bundle only; never mutates the project's existing databases.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const norm = v => String(v ?? '').normalize('NFKC').toLowerCase().replace(/\s+/g, ' ').trim();
const cleanSeason = s => String(s || '').replace(/\s*\([^)]*\bdub\b[^)]*\)\s*/gi, ' ').trim();
const episodeKey = e => JSON.stringify([String(e.episode_number ?? e.episode ?? ''), norm(e.title)]);
const metadata = row => row.panel?.episode_metadata;

function match(history, catalog) {
  if (!history.complete || history.next_page !== null || history.error || !Array.isArray(history.data)) throw new Error('A complete history export is required.');
  if (!Array.isArray(catalog.seasons)) throw new Error('Expected a season catalog export.');
  const byHistory = new Map(history.data.map(h => [h.id, h]));
  if (byHistory.size !== history.data.length) throw new Error('Duplicate history IDs require reconciliation.');
  const buckets = new Map(), badCatalogs = [], aliasConflicts = [], recovered = new Set(), matchedHistory = new Set();
  for (const s of catalog.seasons) {
    if (!s.complete || s.total !== s.data?.length || !s.data?.length || new Set(s.data.map(e=>e.id)).size !== s.data.length) { badCatalogs.push(s.requested_season_id); continue; }
    const all = s.data;
    const episodes = all.filter(e=>!e.is_clip);
    if (!episodes.length || new Set(all.map(e=>e.series_id)).size !== 1) { badCatalogs.push(s.requested_season_id); continue; }
    if (new Set(episodes.map(episodeKey)).size !== episodes.length) { badCatalogs.push(s.requested_season_id); continue; }
    // Only collapse language catalogs when the complete numbered/title episode sets agree.
    // Never merge seasons solely on version links, season numbers, or title similarity.
    const signature = JSON.stringify([episodes[0].series_id, episodes.map(episodeKey).sort()]);
    if (!buckets.has(signature)) buckets.set(signature, {
      key: crypto.createHash('sha256').update(signature).digest('hex').slice(0,16),
      series_id: episodes[0].series_id, series_title: episodes[0].series_title,
      season_title: cleanSeason(episodes[0].season_title), season_number: episodes[0].season_number, source_season_ids: new Set(),
      actual_season_ids: new Set(), episodes: new Map(), audio_locales: new Set(), subtitle_locales: new Set(),
      flags: new Set(),
    });
    const b = buckets.get(signature); b.source_season_ids.add(s.requested_season_id);
    for (const e of episodes) {
      b.actual_season_ids.add(e.season_id);
      if (e.audio_locale) b.audio_locales.add(e.audio_locale);
      for (const l of e.subtitle_locales || []) b.subtitle_locales.add(l);
      const k = episodeKey(e);
      if (!b.episodes.has(k)) b.episodes.set(k, { title:e.title, episode_number:e.episode_number, episode_label:e.episode, source_ids:new Set(), matched_history_ids:new Set(), fully_watched:false, duration_ms:e.duration_ms, evidence:[] });
      const ep = b.episodes.get(k); ep.source_ids.add(e.id);
      const ids = new Set([e.id, ...(e.versions || []).map(v=>v.guid).filter(Boolean)]);
      for (const id of ids) {
        const h = byHistory.get(id); if (!h) continue;
        const hm = metadata(h);
        // Even direct IDs can have inconsistent catalog/history metadata. Quarantine contradictions.
        const conflict = hm && (hm.series_id !== e.series_id || norm(h.panel.title) !== norm(e.title) || (id!==e.id && String(hm.episode_number ?? hm.episode ?? '') !== String(e.episode_number ?? e.episode ?? '')));
        if (conflict) {
          aliasConflicts.push({catalog_episode_id:e.id,history_id:id,catalog_title:e.title,history_title:h.panel?.title,direct:id===e.id});
          if (id===e.id) b.flags.add('direct_id_metadata_conflict');
          continue;
        }
        // Recover missing panels only through exact catalog IDs, not unverified aliases.
        if (!hm && id!==e.id) continue;
        if (!hm) recovered.add(id);
        ep.matched_history_ids.add(id); matchedHistory.add(id);
        if (h.fully_watched===true) ep.fully_watched=true;
      }
    }
  }
  const seasons = [...buckets.values()];
  // Partial-overlap catalogs may be cuts or differently partitioned seasons, not language variants.
  const ownership = new Map();
  for (const b of seasons) for (const e of b.episodes.values()) for (const id of e.source_ids) {
    if (!ownership.has(id)) ownership.set(id,new Set()); ownership.get(id).add(b);
  }
  for (const owners of ownership.values()) if (owners.size>1) for (const b of owners) b.flags.add('overlapping_catalogs_require_review');
  // Prevent a history ID from counting toward two distinct content groups through bad version links.
  const historyOwnership = new Map();
  for (const b of seasons) for (const e of b.episodes.values()) for (const id of e.matched_history_ids) {
    if (!historyOwnership.has(id)) historyOwnership.set(id,new Set()); historyOwnership.get(id).add(b);
  }
  for (const owners of historyOwnership.values()) if (owners.size>1) for (const b of owners) b.flags.add('shared_history_between_seasons');
  const reviews = seasons.map(b=>{
    const eps=[...b.episodes.values()].map(e=>({...e,source_ids:[...e.source_ids],matched_history_ids:[...e.matched_history_ids],evidence:undefined}));
    const watched=eps.filter(e=>e.fully_watched).length;
    const decision=/black clover/i.test(b.series_title)?'excluded_user_unfinished':b.flags.size?'review':watched*5>=eps.length*4?'include':'below_80_percent';
    const generic=/^(season\s*\d+|s\d+|ova|specials?|movies?)$/i.test(b.season_title);
    const title=generic?`${b.series_title} — ${b.season_title}`:b.season_title || b.series_title;
    return {key:b.key,series_id:b.series_id,series_title:b.series_title,title,season_title:b.season_title,season_number:b.season_number,source_season_ids:[...b.source_season_ids],actual_season_ids:[...b.actual_season_ids],audio_locales:[...b.audio_locales].sort(),subtitle_locales:[...b.subtitle_locales].sort(),episode_count:eps.length,fully_watched_episode_count:watched,watched_fraction:watched/eps.length,decision,flags:[...b.flags],min_runtime:eps.every(e=>Number.isFinite(e.duration_ms)&&e.duration_ms>0)?Math.round(eps.reduce((n,e)=>n+e.duration_ms,0)/60000):null,episodes:eps};
  }).sort((a,b)=>a.series_title.localeCompare(b.series_title)||a.title.localeCompare(b.title)||a.key.localeCompare(b.key));
  const included=reviews.filter(s=>s.decision==='include');
  const titleCounts=new Map();for(const s of included)titleCounts.set(s.title,(titleCounts.get(s.title)||0)+1);
  for(const s of included)if(titleCounts.get(s.title)>1)s.title+=` — Season ${s.season_number ?? s.actual_season_ids[0]}`;
  const groups=[],groupIds=new Map();
  for(const s of included)if(!groupIds.has(s.series_id)){groupIds.set(s.series_id,groups.length);groups.push({id:groups.length,rank:0,name:s.series_title,description:'',members:[],source_series_id:s.series_id});}
  const entries=included.map((s,id)=>{
    const ip_group=groupIds.get(s.series_id);groups[ip_group].members.push(id);
    return {id,rank:0,title:s.title,episode_count:s.episode_count,min_runtime:s.min_runtime,ip_group,source_key:s.key,
      // Unknown release status, format, and lookup values must not become fabricated facts.
      unresolved_fields:['format','status','genres','fan_service'],source_audio_locales:s.audio_locales,source_subtitle_locales:s.subtitle_locales};
  });
  const unresolved=history.data.filter(h=>!metadata(h)&&!recovered.has(h.id));
  const counts={};for(const s of reviews)counts[s.decision]=(counts[s.decision]||0)+1;
  return {summary:{history_records:history.data.length,catalog_requests:catalog.seasons.length,invalid_catalog_requests:badCatalogs.length,consolidated_seasons:reviews.length,decisions:counts,proposed_entries:entries.length,proposed_groups:groups.length,missing_metadata_recovered:recovered.size,missing_metadata_remaining:unresolved.length,unmatched_history_records:history.data.length-matchedHistory.size,conflicting_alias_comparisons:aliasConflicts.length,threshold:0.8},entries,groups,seasons:reviews,unresolved_history:unresolved,unmatched_history:history.data.filter(h=>!matchedHistory.has(h.id)),conflicts:aliasConflicts,invalid_catalogs:badCatalogs};
}
if(require.main===module){
  const [historyPath,catalogPath,out='out/crunchyroll/final']=process.argv.slice(2);
  if(!historyPath||!catalogPath)throw new Error('Provide history and catalog JSON paths.');
  const read=p=>JSON.parse(fs.readFileSync(p,'utf8').replace(/^\uFEFF/,''));const r=match(read(historyPath),read(catalogPath));fs.mkdirSync(out,{recursive:true});
  for(const [name,value]of Object.entries(r))fs.writeFileSync(path.join(out,`${name}.json`),JSON.stringify(value,null,2));
  const cell=s=>String(s).replace(/\|/g,'\\|').replace(/[\r\n]/g,' ');
  const md=['# Crunchyroll import candidates','',`**${r.summary.proposed_entries} entries in ${r.summary.proposed_groups} groups qualify.**`,'',
    'This is a review bundle, not an update to the live database. IDs are provisional within this bundle and must be remapped when merging existing entries. Release status, format, genres, and fan-service ratings remain unresolved. Source language codes have not been converted to project lookup IDs.','',
    'A season qualifies when at least 80% of its catalog episodes have a matching fully_watched history record. Partial playback is retained in the source history but does not count as fully watched. Clips are excluded. The denominator is the catalog snapshot, not a claim that the anime has finished airing. Exact numbered/title episode sets consolidate duplicate language catalogs. Conflicting alias links do not count. Black Clover is excluded by user instruction.','',
    'Only seasons represented in the history were queried. Related movies and separate series are not automatically combined into a broader franchise.','',
    `Unresolved metadata: ${r.summary.missing_metadata_remaining} history records; ${r.summary.missing_metadata_recovered} recovered through direct catalog IDs.`,'',
    '| Title | Watched / catalog | Percentage | Decision |','|---|---:|---:|---|',
    ...r.seasons.map(s=>`| ${cell(s.title)} | ${s.fully_watched_episode_count}/${s.episode_count} | ${(100*s.watched_fraction).toFixed(1)}% | ${s.decision} |`)];
  fs.writeFileSync(path.join(out,'review.md'),md.join('\n')+'\n');console.log(JSON.stringify(r.summary,null,2));
}
module.exports={match};

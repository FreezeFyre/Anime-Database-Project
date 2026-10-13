// Usage: node scripts/prepare-crunchyroll-import.cjs <export.json> [output-directory]
// Produces review candidates, not database records: catalog denominators are required.
const fs = require('node:fs');
const path = require('node:path');

function prepare(source) {
  if (!Array.isArray(source.data)) throw new Error('Expected an export with a data array.');
  const groups = new Map(), seasons = new Map(), seen = new Set();
  const unresolved = [], duplicateIds = [], mismatches = [];
  for (const row of source.data) {
    if (!row.id) throw new Error('History record has no episode ID.');
    if (seen.has(row.id)) { duplicateIds.push(row.id); continue; }
    seen.add(row.id);
    const m = row.panel?.episode_metadata;
    if (!m?.series_id || !m.season_id) { unresolved.push(row); continue; }
    if (row.parent_id !== m.series_id) mismatches.push({ episode_id: row.id, parent_id: row.parent_id, series_id: m.series_id });
    if (!groups.has(m.series_id)) groups.set(m.series_id, {
      source_series_id: m.series_id, name: m.series_title,
      source_parent_ids: new Set(), source_season_ids: new Set(),
      project_group_id: null,
    });
    const group = groups.get(m.series_id);
    group.source_parent_ids.add(row.parent_id);
    group.source_season_ids.add(m.season_id);
    const key = `${m.series_id}/${m.season_id}`;
    if (!seasons.has(key)) seasons.set(key, {
      source_series_id: m.series_id, source_season_id: m.season_id,
      series_title: m.series_title, season_title: m.season_title,
      season_number: m.season_number, season_sequence_number: m.season_sequence_number,
      project_entry_id: null, catalog_episode_count: null, watched_fraction: null,
      import_decision: /black clover/i.test(m.series_title || '') ? 'excluded_user_unfinished' : 'pending_catalog_and_variant_resolution',
      episodes: [],
    });
    seasons.get(key).episodes.push({
      source_episode_id: row.id, title: row.panel.title, episode_number: m.episode_number,
      episode_label: m.episode, sequence_number: m.sequence_number,
      fully_watched: row.fully_watched === true, playhead_seconds: row.playhead,
      duration_ms: m.duration_ms, date_played: row.date_played,
      audio_locale: m.audio_locale, subtitle_locales: m.subtitle_locales,
      is_clip: m.is_clip, episode_air_date: m.episode_air_date,
      description: row.panel.description,
      // Keep version evidence without assuming these links are correct.
      versions: m.versions,
    });
  }
  const entries = [...seasons.values()].sort((a,b) => a.series_title.localeCompare(b.series_title) || a.season_sequence_number-b.season_sequence_number || a.source_season_id.localeCompare(b.source_season_id));
  for (const season of entries) {
    season.history_records = season.episodes.length;
    season.fully_watched_records = season.episodes.filter(e => e.fully_watched && !e.is_clip).length;
    season.audio_locales = [...new Set(season.episodes.map(e=>e.audio_locale).filter(Boolean))].sort();
  }
  const normalizedGroups = [...groups.values()].map(g=>({...g,source_parent_ids:[...g.source_parent_ids],source_season_ids:[...g.source_season_ids]})).sort((a,b)=>a.name.localeCompare(b.name));
  const dates = source.data.map(r=>r.date_played).filter(Boolean).sort();
  const pageCounts = source.pages?.reduce((sum,p)=>sum+p.count,0);
  const audit = {
    complete: source.complete === true && source.next_page === null && !source.error && pageCounts === source.data.length,
    pages: source.pages?.length ?? null, records: source.data.length, unique_episode_ids: seen.size,
    duplicate_ids: duplicateIds, oldest_utc: dates[0], newest_utc: dates.at(-1),
    fully_watched_records: source.data.filter(r=>r.fully_watched===true).length,
    nonzero_playheads: source.data.filter(r=>r.playhead>0).length,
    candidate_groups: normalizedGroups.length, source_seasons_including_language_variants: entries.length,
    unresolved_records: unresolved.length, parent_series_mismatches: mismatches,
    season_completion_threshold: 0.8,
    notes: [
      'Candidates only. No database records have been written.',
      'Counts are source episode records, not canonical episodes; language variants remain separate.',
      'Catalog episode lists are required for denominators. Highest episode number is not a season total.',
      'fully_watched is preserved; partial plays are not automatically counted as completed episodes.',
      'Black Clover is excluded because the user explicitly said it is unfinished.',
      'Project numeric IDs, lookup values, and release status are not inferred from viewing history.',
    ],
  };
  return {audit, groups:normalizedGroups, entries, unresolved};
}

if (require.main === module) {
  const input = process.argv[2], out = process.argv[3] || 'out/crunchyroll/prepared';
  if (!input) throw new Error('Provide the downloaded history JSON path.');
  const raw = fs.readFileSync(input,'utf8');
  const result = prepare(JSON.parse(raw.replace(/^\uFEFF/,'')));
  fs.mkdirSync(out,{recursive:true});
  for (const [name,value] of Object.entries(result)) fs.writeFileSync(path.join(out,`${name}.json`),JSON.stringify(value,null,2));
  fs.writeFileSync(path.join(out,'history-source.json'),raw);
  const cell = s => String(s??'').replace(/\|/g,'\\|').replace(/[\r\n]/g,' ');
  const lines = [
    '# Crunchyroll import review', '',
    `${result.audit.records} history records; ${result.audit.candidate_groups} candidate groups; ${result.audit.source_seasons_including_language_variants} source seasons (including language variants).`, '',
    ...result.audit.notes.map(n=>`- ${n}`), '',
    '| Series | Season | Audio | History records | Fully watched records | Decision |',
    '|---|---|---|---:|---:|---|',
    ...result.entries.map(e=>`| ${cell(e.series_title)} | ${cell(e.season_title)} | ${cell(e.audio_locales.join(', '))} | ${e.history_records} | ${e.fully_watched_records} | ${e.import_decision} |`),
  ];
  fs.writeFileSync(path.join(out,'review.md'),lines.join('\n')+'\n');
  console.log(JSON.stringify({...result.audit,parent_series_mismatches:result.audit.parent_series_mismatches.length},null,2));
}
module.exports = { prepare };

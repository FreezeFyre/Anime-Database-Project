// Run in the signed-in Crunchyroll console. Copy as Fetch a successful season
// episodes GET request and replace its leading fetch( with exportCrunchyrollCatalog(.
// Then choose your downloaded history JSON in the panel. Request credentials stay
// in this tab and are never included in the downloaded catalog.
window.exportCrunchyrollCatalog = async (input, requestOptions = {}) => {
  const origin = 'https://www.crunchyroll.com';
  const template = new URL(input, origin);
  const route = /^\/content\/v2\/cms\/seasons\/[A-Za-z0-9_-]+\/episodes$/;
  if (location.origin !== origin || template.origin !== origin || !route.test(template.pathname) || template.username || template.password) {
    throw new Error('Use a Crunchyroll season episodes request on www.crunchyroll.com.');
  }
  if (requestOptions.method && requestOptions.method.toUpperCase() !== 'GET') throw new Error('Use a GET request.');
  if (document.getElementById('cr-catalog-export')) throw new Error('Reload before installing another catalog exporter.');
  const headers = new Headers(requestOptions.headers);
  headers.set('Accept', 'application/json');
  const panel = document.createElement('div');
  panel.id = 'cr-catalog-export';
  panel.style.cssText = 'position:fixed;top:16px;right:16px;z-index:2147483647;background:white;color:#111;padding:20px;max-width:480px;font:15px/1.5 sans-serif;border:3px solid #f47521;border-radius:8px;box-shadow:0 4px 24px #0008';
  const addFile = (label) => {
    const p = document.createElement('p'); p.textContent = label;
    const file = document.createElement('input'); file.type = 'file'; file.accept = '.json,application/json';
    panel.append(p, file); return file;
  };
  const historyFile = addFile('Choose your complete history JSON:');
  const resumeFile = addFile('Optional: previous partial catalog, to resume:');
  const status = document.createElement('p'); status.textContent = 'Choose the history file, then Start.';
  const start = document.createElement('button'); start.textContent = 'Start';
  const stop = document.createElement('button'); stop.textContent = 'Stop after current request'; stop.disabled = true;
  const save = document.createElement('button'); save.textContent = 'Download catalog'; save.disabled = true;
  panel.append(status, start, stop, save); document.body.append(panel);
  let stopped = false;
  const result = { source: 'Crunchyroll season episode catalog', export_version: 1, complete: false, started_at: new Date().toISOString(), requested_season_ids: [], seasons: [], error: null };
  const delay = ms => new Promise(resolve => setTimeout(resolve, ms));
  const parseFile = async file => JSON.parse((await file.text()).replace(/^\uFEFF/, ''));
  stop.onclick = () => { stopped = true; stop.disabled = true; };
  save.onclick = () => {
    const url = URL.createObjectURL(new Blob([JSON.stringify(result, null, 2)], { type: 'application/json' }));
    const link = document.createElement('a'); link.href = url;
    link.download = `crunchyroll-catalog-${result.complete ? 'complete' : 'partial'}-${new Date().toISOString().slice(0,10)}.json`;
    document.body.append(link); link.click(); link.remove(); setTimeout(() => URL.revokeObjectURL(url), 60000);
  };
  start.onclick = async () => {
    if (!historyFile.files[0]) { status.textContent = 'Choose your history JSON first.'; return; }
    start.disabled = true;
    try {
      const history = await parseFile(historyFile.files[0]);
      if (!Array.isArray(history.data)) throw new Error('Not a history export.');
      const ids = [...new Set(history.data.map(r => r.panel?.episode_metadata?.season_id).filter(Boolean))].sort();
      if (!ids.length || ids.some(id => !/^[A-Za-z0-9_-]+$/.test(id))) throw new Error('No valid season IDs found.');
      result.requested_season_ids = ids;
      if (resumeFile.files[0]) {
        const previous = await parseFile(resumeFile.files[0]);
        if (previous.source !== result.source || !Array.isArray(previous.seasons)) throw new Error('Not a catalog export.');
        const allowed = new Set(ids), restored = new Set();
        result.seasons = previous.seasons.filter(s => {
          if (!allowed.has(s.requested_season_id) || restored.has(s.requested_season_id) || s.complete !== true || !Array.isArray(s.data) || !s.data.length || s.total !== s.data.length) return false;
          restored.add(s.requested_season_id); return true;
        });
      }
      historyFile.disabled = resumeFile.disabled = true;
      save.disabled = false; stop.disabled = false;
      const done = new Set(result.seasons.map(s => s.requested_season_id));
      for (const id of ids) {
        if (stopped) break;
        if (done.has(id)) continue;
        status.textContent = `${result.seasons.length}/${ids.length} seasons processed. Fetching ${id}…`;
        const url = new URL(template.href);
        url.pathname = `/content/v2/cms/seasons/${id}/episodes`;
        // The supplied endpoint returns a complete list. Never carry a copied page cursor.
        for (const param of ['page', 'offset', 'start']) url.searchParams.delete(param);
        const record = { requested_season_id: id, data: [], total: null, complete: false, error: null };
        const visited = new Set();
        let next = url.href;
        try {
          while (next && !stopped) {
            const current = new URL(next, origin);
            if (current.origin !== origin || current.pathname !== url.pathname || current.username || current.password) throw new Error('Pagination left the requested season endpoint.');
            if (visited.has(current.href) || visited.size >= 1000) throw new Error('Repeated cursor or pagination limit.');
            let response;
            for (let attempt = 0; attempt < 5; attempt++) {
              response = await fetch(current.href, {
                ...requestOptions, method: 'GET', body: undefined, credentials: 'same-origin',
                redirect: 'error', cache: 'no-store', headers, signal: AbortSignal.timeout(45000),
              });
              if (![429, 502, 503, 504].includes(response.status) || attempt === 4) break;
              const retry = response.headers.get('Retry-After');
              const wait = retry && /^\d+$/.test(retry) ? Number(retry)*1000 : retry && Number.isFinite(Date.parse(retry)) ? Math.max(0,Date.parse(retry)-Date.now()) : 2000*2**attempt;
              status.textContent = `Server busy (${response.status}); retrying in ${Math.ceil(wait/1000)} seconds…`;
              await delay(wait); if (stopped) break;
            }
            if (stopped && !response?.ok) break;
            if ([401, 403, 429].includes(response.status)) {
              stopped = true;
              throw new Error(`HTTP ${response.status}: download the partial catalog. Resume after refreshing the successful request or waiting for the rate limit.`);
            }
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            const page = await response.json();
            if (!Array.isArray(page.data) || page.data.some(e => !e.id || !e.season_id)) throw new Error('Unexpected catalog response shape.');
            if (!Number.isInteger(page.total) || page.total < 0) throw new Error('Missing catalog total.');
            if (record.total === null) record.total = page.total;
            record.data.push(...page.data);
            visited.add(current.href);
            const cursor = page.meta?.next_page;
            if (cursor != null && typeof cursor !== 'string') throw new Error('Invalid pagination value.');
            next = cursor ? new URL(cursor, origin).href : null;
            if (next && !stopped) await delay(750);
          }
          const unique = new Set(record.data.map(e => e.id));
          record.complete = next === null && record.data.length > 0 && unique.size === record.data.length && record.total === record.data.length;
          if (!record.complete) record.error = 'Empty, partial, duplicate, or count-mismatched episode list; requires review.';
        } catch (error) {
          record.error = String(error.message || error);
          if (stopped) result.error = record.error;
        }
        result.seasons.push(record);
        if (!stopped) await delay(750);
      }
      result.finished_at = new Date().toISOString();
      const resolved = new Set(result.seasons.filter(s => s.complete).map(s => s.requested_season_id));
      result.complete = ids.every(id => resolved.has(id));
      status.textContent = `${result.complete ? 'Complete' : 'Partial — some seasons need retry/review'}: ${resolved.size}/${ids.length} valid season lists. Download the catalog.${result.error ? ' '+result.error : ''}`;
    } catch (error) {
      result.error = String(error.message || error); status.textContent = result.error;
      save.disabled = false;
    } finally { stop.disabled = true; }
  };
};
console.info('Catalog exporter installed. Copy as Fetch a successful season episodes GET request, replace leading fetch( with exportCrunchyrollCatalog(, and run it here. Keep request credentials in Firefox.');

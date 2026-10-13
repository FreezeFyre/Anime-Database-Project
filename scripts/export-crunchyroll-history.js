// Run in Firefox's Web Console on your signed-in https://www.crunchyroll.com/history tab.
// Then Copy as Fetch a successful history request in Network, paste it into the
// console, and replace ONLY its leading fetch( with exportCrunchyrollHistory(.
// Keep that copied request in Firefox: it may contain authorization credentials.
// Request headers are reused only for the original endpoint and never exported.
window.exportCrunchyrollHistory = async (input, requestOptions = {}) => {
  const origin = "https://www.crunchyroll.com";
  if (location.origin !== origin) throw new Error("Run this on www.crunchyroll.com.");
  if (document.getElementById("cr-history-export")) {
    throw new Error("An export panel already exists. Finish it or reload before restarting.");
  }
  if (!input) return;
  const first = new URL(input, origin);
  if (first.origin !== origin || !/^\/content\/v2\/[^/]+\/watch-history$/.test(first.pathname)) {
    throw new Error("Expected the same-site /content/v2/<profile>/watch-history URL.");
  }
  if (requestOptions.method && requestOptions.method.toUpperCase() !== "GET") {
    throw new Error("Copy a GET history request.");
  }
  // Start at the beginning even when the copied request was a later history page.
  first.searchParams.delete("page");
  first.searchParams.set("page_size", "100");
  const headers = new Headers(requestOptions.headers);
  headers.set("Accept", "application/json");
  const validateNext = (value) => {
    const url = new URL(value, origin);
    if (url.origin !== origin || url.pathname !== first.pathname || url.username || url.password) {
      throw new Error("Stopped: pagination left the original history endpoint.");
    }
    url.searchParams.set("page_size", "100");
    return url.href;
  };
  const panel = document.createElement("div");
  panel.id = "cr-history-export";
  panel.style.cssText = "position:fixed;top:16px;right:16px;z-index:2147483647;background:#fff;color:#111;padding:20px;max-width:440px;font:15px/1.5 sans-serif;border:3px solid #f47521;border-radius:8px;box-shadow:0 4px 24px #0008";
  const status = document.createElement("p");
  status.textContent = "Starting history export…";
  const stop = document.createElement("button");
  stop.textContent = "Stop after current page";
  const save = document.createElement("button");
  save.textContent = "Download partial JSON";
  save.style.marginLeft = "8px";
  panel.append(status, stop, save);
  document.body.append(panel);
  let stopped = false;
  let next = validateNext(first.href);
  const visited = new Set();
  const result = {
    source: "Crunchyroll watch-history",
    export_version: 2,
    started_at: new Date().toISOString(),
    complete: false,
    pages: [],
    data: [],
    next_page: next,
    error: null,
  };
  const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
  stop.onclick = () => { stopped = true; stop.disabled = true; };
  save.onclick = () => {
    const blob = new Blob([JSON.stringify(result, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `crunchyroll-history-${result.complete ? "complete" : "partial"}-${new Date().toISOString().slice(0, 10)}.json`;
    document.body.append(link);
    link.click();
    link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 60000);
  };
  try {
    while (next && !stopped) {
      if (visited.has(next)) throw new Error("Repeated pagination cursor; stopped to avoid a loop.");
      if (visited.size >= 1000) throw new Error("Reached the 1,000-page safety limit.");
      let response;
      for (let attempt = 0; attempt < 5; attempt++) {
        response = await fetch(next, {
          ...requestOptions,
          method: "GET", credentials: "same-origin", redirect: "error",
          body: undefined, cache: "no-store", headers, signal: AbortSignal.timeout(45000),
        });
        if (![429, 502, 503, 504].includes(response.status) || attempt === 4) break;
        const retry = response.headers.get("Retry-After");
        const seconds = retry && /^\d+$/.test(retry) ? Number(retry) : NaN;
        const wait = Number.isFinite(seconds) ? seconds * 1000 :
          retry && Number.isFinite(Date.parse(retry)) ? Math.max(0, Date.parse(retry) - Date.now()) :
          2000 * (2 ** attempt);
        status.textContent = `Server busy (${response.status}). Retrying in ${Math.ceil(wait / 1000)} seconds…`;
        await delay(wait);
        if (stopped) break;
      }
      if (stopped && !response?.ok) break;
      if (!response.ok) throw new Error(`HTTP ${response.status}. For 401/403, download any partial results, reload, reinstall this script, and use Copy as Fetch from a fresh successful history request. Keep the copied request in Firefox.`);
      const page = await response.json();
      if (!Array.isArray(page.data) || !page.meta || !("next_page" in page.meta)) {
        throw new Error("Unexpected response shape; cannot verify pagination.");
      }
      const rawNext = page.meta.next_page;
      if (rawNext !== null && typeof rawNext !== "string") throw new Error("Invalid next_page value.");
      const nextUrl = rawNext ? validateNext(rawNext) : null;
      result.data.push(...page.data);
      result.pages.push({ count: page.data.length, reported_total: page.total ?? null });
      visited.add(next);
      next = nextUrl;
      result.next_page = next;
      status.textContent = `Collected ${result.data.length} records across ${result.pages.length} pages…`;
      if (next && !stopped) await delay(750);
    }
    result.complete = next === null;
    result.finished_at = new Date().toISOString();
    status.textContent = `${result.complete ? "Complete" : "Stopped — partial export"}: ${result.data.length} records across ${result.pages.length} pages. Click Download to save.`;
  } catch (error) {
    result.error = String(error.message || error);
    status.textContent = `${result.error} Saved in memory: ${result.data.length} records. Download the partial file before reloading.`;
    console.error("Crunchyroll history export:", result.error);
  } finally {
    stop.disabled = true;
    save.textContent = result.complete ? "Download complete JSON" : "Download partial JSON";
  }
};
console.info("Exporter installed. Copy as Fetch a successful history GET request, change its leading fetch( to exportCrunchyrollHistory(, and run it here. Do not share the copied request.");

const ENCODE_VERSION = 1001;

const COM_TYPE = {
  REQ: 1,
  RESP: 2,
};

const DOMAIN_TYPE = {
  ENTRY: 1,
};

const ACTION_TYPE = {
  ACT_GET: 1,
  ACT_FILTER: 8,
};

const FILTER_FLAGS = {
  INCLUDE_ALL: 1 << 0,
  INCLUDE_ANY: 1 << 1,
  EXCLUDE: 1 << 2,
  RANGE_MIN: 1 << 3,
  RANGE_MAX: 1 << 4,
};

const DATA_TYPES = {
  0: "U8",
  1: "U16",
  2: "U32",
  3: "STR",
  4: "VECU8",
  5: "VECU16",
};

const fields = {
  form: document.querySelector("#request-form"),
  filterForm: document.querySelector("#filter-form"),
  serverUrl: document.querySelector("#server-url"),
  entryIds: document.querySelector("#entry-ids"),
  filterField: document.querySelector("#filter-field"),
  filterAny: document.querySelector("#filter-any"),
  filterMin: document.querySelector("#filter-min"),
  filterMax: document.querySelector("#filter-max"),
  sendButton: document.querySelector("#send-button"),
  filterButton: document.querySelector("#filter-button"),
  clearButton: document.querySelector("#clear-button"),
  status: document.querySelector("#status"),
  messageId: document.querySelector("#message-id"),
  recordCount: document.querySelector("#record-count"),
  payloadSize: document.querySelector("#payload-size"),
  filteredIds: document.querySelector("#filtered-ids"),
  entriesBody: document.querySelector("#entries-body"),
  rawOutput: document.querySelector("#raw-output"),
};

let nextMessageId = 1;
let cachedEntries = [];

fields.form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const messageId = nextMessageId++;

  setStatus("loading", "Connecting");
  fields.sendButton.disabled = true;
  fields.messageId.textContent = String(messageId);

  try {
    const ids = parseIds(fields.entryIds.value);
    const response = await sendRequest(fields.serverUrl.value.trim(), ACTION_TYPE.ACT_GET, encodeVecU32(ids), messageId);
    const decoded = decodeEntriesPayload(response.payload);
    cachedEntries = decoded.entries;
    renderEntries(decoded.entries);
    renderRaw(response, decoded);
    fields.recordCount.textContent = String(decoded.entries.length);
    fields.payloadSize.textContent = `${response.payloadSize} bytes`;
    fields.filteredIds.textContent = "-";
    setStatus("success", "Received");
  } catch (error) {
    setStatus("error", "Error");
    renderError(error);
  } finally {
    fields.sendButton.disabled = false;
  }
});

fields.filterForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  const messageId = nextMessageId++;

  setStatus("loading", "Filtering");
  fields.filterButton.disabled = true;
  fields.messageId.textContent = String(messageId);

  try {
    const filter = readFilterForm();
    const payload = encodeFilterRequest([filter]);
    const response = await sendRequest(fields.serverUrl.value.trim(), ACTION_TYPE.ACT_FILTER, payload, messageId);
    const ids = decodeVecU32(response.payload);
    const visibleEntries = cachedEntries.length > 0
      ? cachedEntries.filter((entry) => ids.includes(Number(entry.id)))
      : [];

    fields.filteredIds.textContent = ids.length > 0 ? ids.join(", ") : "(none)";
    fields.recordCount.textContent = cachedEntries.length > 0 ? String(visibleEntries.length) : "0";
    fields.payloadSize.textContent = `${response.payloadSize} bytes`;
    renderEntries(visibleEntries);
    renderFilterRaw(response, filter, ids, visibleEntries);
    setStatus("success", "Filtered");
  } catch (error) {
    setStatus("error", "Error");
    renderError(error);
  } finally {
    fields.filterButton.disabled = false;
  }
});

fields.clearButton.addEventListener("click", () => {
  cachedEntries = [];
  renderEntries([]);
  fields.rawOutput.textContent = "Waiting for a request...";
  fields.messageId.textContent = "-";
  fields.recordCount.textContent = "0";
  fields.payloadSize.textContent = "0 bytes";
  fields.filteredIds.textContent = "-";
  setStatus("idle", "Idle");
});

function sendRequest(url, action, payload, messageId) {
  return new Promise((resolve, reject) => {
    const socket = new WebSocket(url);
    const timeout = window.setTimeout(() => {
      socket.close();
      reject(new Error("Timed out waiting for the server response."));
    }, 8000);

    socket.binaryType = "arraybuffer";

    socket.addEventListener("open", () => {
      setStatus("loading", "Sending");
      socket.send(encodeMessage({
        comType: COM_TYPE.REQ,
        domain: DOMAIN_TYPE.ENTRY,
        action,
        id: messageId,
        payload,
      }));
    });

    socket.addEventListener("message", (event) => {
      window.clearTimeout(timeout);
      try {
        resolve(decodeMessage(new Uint8Array(event.data)));
      } catch (error) {
        reject(error);
      } finally {
        socket.close();
      }
    });

    socket.addEventListener("error", () => {
      window.clearTimeout(timeout);
      reject(new Error(`Could not connect to ${url}. Make sure the server is running.`));
    });

    socket.addEventListener("close", (event) => {
      if (!event.wasClean && fields.status.textContent !== "Received") {
        window.clearTimeout(timeout);
      }
    });
  });
}

function encodeMessage(message) {
  const payloadSize = message.payload.length;
  const writer = new ByteWriter(15 + payloadSize);
  writer.u32(ENCODE_VERSION);
  writer.u8(message.comType);
  writer.u8(message.domain);
  writer.u8(message.action);
  writer.u32(message.id);
  writer.u32(payloadSize);
  writer.bytes(message.payload);
  return writer.buffer;
}

function decodeMessage(bytes) {
  const reader = new ByteReader(bytes);
  const encodeVersion = reader.u32();
  if (encodeVersion !== ENCODE_VERSION) {
    throw new Error(`Unsupported encode version: ${encodeVersion}`);
  }

  const message = {
    encodeVersion,
    comType: reader.u8(),
    domain: reader.u8(),
    action: reader.u8(),
    id: reader.u32(),
    payloadSize: reader.u32(),
  };

  message.payload = reader.bytes(message.payloadSize);
  return message;
}

function encodeVecU32(values) {
  const writer = new ByteWriter(4 + values.length * 4);
  writer.u32(values.length);
  values.forEach((value) => writer.u32(value));
  return new Uint8Array(writer.buffer);
}

function decodeVecU32(payload) {
  const reader = new ByteReader(payload);
  return reader.vecU32();
}

function encodeFilterRequest(filters) {
  const encoder = new TextEncoder();
  let size = 4;
  const prepared = filters.map((filter) => {
    const nameBytes = encoder.encode(filter.name);
    let flags = 0;
    if (filter.includeAll.length > 0) flags |= FILTER_FLAGS.INCLUDE_ALL;
    if (filter.includeAny.length > 0) flags |= FILTER_FLAGS.INCLUDE_ANY;
    if (filter.exclude.length > 0) flags |= FILTER_FLAGS.EXCLUDE;
    if (filter.range.min !== null) flags |= FILTER_FLAGS.RANGE_MIN;
    if (filter.range.max !== null) flags |= FILTER_FLAGS.RANGE_MAX;

    size += 4 + nameBytes.length + 1;
    if (flags & FILTER_FLAGS.INCLUDE_ALL) size += 4 + filter.includeAll.length * 4;
    if (flags & FILTER_FLAGS.INCLUDE_ANY) size += 4 + filter.includeAny.length * 4;
    if (flags & FILTER_FLAGS.EXCLUDE) size += 4 + filter.exclude.length * 4;
    if (flags & FILTER_FLAGS.RANGE_MIN) size += 4;
    if (flags & FILTER_FLAGS.RANGE_MAX) size += 4;

    return { ...filter, flags, nameBytes };
  });

  const writer = new ByteWriter(size);
  writer.u32(prepared.length);
  prepared.forEach((filter) => {
    writer.strBytes(filter.nameBytes);
    writer.u8(filter.flags);
    if (filter.flags & FILTER_FLAGS.INCLUDE_ALL) writer.vecU32(filter.includeAll);
    if (filter.flags & FILTER_FLAGS.INCLUDE_ANY) writer.vecU32(filter.includeAny);
    if (filter.flags & FILTER_FLAGS.EXCLUDE) writer.vecU32(filter.exclude);
    if (filter.flags & FILTER_FLAGS.RANGE_MIN) writer.u32(filter.range.min);
    if (filter.flags & FILTER_FLAGS.RANGE_MAX) writer.u32(filter.range.max);
  });

  return new Uint8Array(writer.buffer);
}

function decodeEntriesPayload(payload) {
  const reader = new ByteReader(payload);
  const section = {
    type: reader.u8(),
    title: reader.str(),
    recordCount: reader.u32(),
    fieldCount: reader.u32(),
    hasSchema: Boolean(reader.u8()),
  };

  const schemaLength = reader.u32();
  const schema = Array.from({ length: schemaLength }, () => ({
    name: reader.str(),
    type: reader.u8(),
    mode: reader.u8(),
  }));

  const entries = [];
  while (reader.remaining() > 0) {
    entries.push(readEntry(reader, schema));
  }

  return { section, schema, entries };
}

function readEntry(reader, schema) {
  const entry = {};
  for (const field of schema) {
    entry[field.name] = readTypedValue(reader, field.type);
  }
  return entry;
}

function readTypedValue(reader, type) {
  switch (DATA_TYPES[type]) {
    case "U8":
      return reader.u8();
    case "U16":
      return reader.u16();
    case "U32":
      return reader.u32();
    case "STR":
      return reader.str();
    case "VECU8":
      return reader.vecU8();
    case "VECU16":
      return reader.vecU16();
    default:
      throw new Error(`Unknown field data type: ${type}`);
  }
}

function parseIds(input) {
  const ids = input
    .split(",")
    .map((part) => part.trim())
    .filter(Boolean)
    .map((part) => {
      const value = Number(part);
      if (!Number.isInteger(value) || value < 0 || value > 0xffffffff) {
        throw new Error(`Invalid entry ID: ${part}`);
      }
      return value;
    });

  if (ids.length === 0) {
    throw new Error("Enter at least one entry ID.");
  }

  return ids;
}

function readFilterForm() {
  const selected = fields.filterField.selectedOptions[0];
  const kind = selected.dataset.kind;
  const filter = {
    name: fields.filterField.value,
    includeAll: [],
    includeAny: [],
    exclude: [],
    range: {
      min: null,
      max: null,
    },
  };

  if (kind === "multi") {
    filter.includeAny = parseOptionalU32List(fields.filterAny.value);
    if (filter.includeAny.length === 0) {
      throw new Error("Enter at least one Include Any value for multi-value filters.");
    }
    return filter;
  }

  filter.range.min = parseOptionalU32(fields.filterMin.value, "Min");
  filter.range.max = parseOptionalU32(fields.filterMax.value, "Max");
  if (filter.range.min === null && filter.range.max === null) {
    throw new Error("Enter a Min, Max, or both for range filters.");
  }
  if (filter.range.min !== null && filter.range.max !== null && filter.range.min > filter.range.max) {
    throw new Error("Min cannot be greater than Max.");
  }
  return filter;
}

function parseOptionalU32List(input) {
  return input
    .split(",")
    .map((part) => part.trim())
    .filter(Boolean)
    .map((part) => parseU32(part, "Filter value"));
}

function parseOptionalU32(input, label) {
  const trimmed = input.trim();
  return trimmed === "" ? null : parseU32(trimmed, label);
}

function parseU32(input, label) {
  const value = Number(input);
  if (!Number.isInteger(value) || value < 0 || value > 0xffffffff) {
    throw new Error(`${label} must be an integer from 0 to 4294967295.`);
  }
  return value;
}

function renderEntries(entries) {
  if (entries.length === 0) {
    fields.entriesBody.innerHTML = '<tr><td colspan="7" class="empty">No entries to display.</td></tr>';
    return;
  }

  fields.entriesBody.innerHTML = entries.map((entry) => `
    <tr>
      <td>${escapeHtml(entry.id)}</td>
      <td>${escapeHtml(entry.rank)}</td>
      <td>${escapeHtml(entry.title || "(untitled)")}</td>
      <td>${escapeHtml(entry.format)}</td>
      <td>${escapeHtml(entry.status)}</td>
      <td>${escapeHtml(entry.episode_count)}</td>
      <td>${escapeHtml(entry.min_runtime)}</td>
    </tr>
  `).join("");
}

function renderRaw(message, decoded) {
  const printable = {
    message: {
      encodeVersion: message.encodeVersion,
      comType: message.comType,
      domain: message.domain,
      action: message.action,
      id: message.id,
      payloadSize: message.payloadSize,
    },
    section: decoded.section,
    schema: decoded.schema.map((field) => ({
      ...field,
      typeName: DATA_TYPES[field.type] || "UNKNOWN",
    })),
    entries: decoded.entries,
  };

  fields.rawOutput.textContent = JSON.stringify(printable, null, 2);
}

function renderFilterRaw(message, filter, ids, visibleEntries) {
  const printable = {
    message: {
      encodeVersion: message.encodeVersion,
      comType: message.comType,
      domain: message.domain,
      action: message.action,
      id: message.id,
      payloadSize: message.payloadSize,
    },
    request: filter,
    filteredIds: ids,
    displayedFromCache: visibleEntries,
  };

  fields.rawOutput.textContent = JSON.stringify(printable, null, 2);
}

function renderError(error) {
  fields.rawOutput.textContent = error.stack || error.message || String(error);
  fields.entriesBody.innerHTML = '<tr><td colspan="7" class="empty">Request failed. Check the decoded response panel.</td></tr>';
}

function setStatus(kind, text) {
  fields.status.className = `status ${kind}`;
  fields.status.textContent = text;
}

function escapeHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

class ByteWriter {
  constructor(size) {
    this.buffer = new ArrayBuffer(size);
    this.view = new DataView(this.buffer);
    this.offset = 0;
  }

  u8(value) {
    this.view.setUint8(this.offset, value);
    this.offset += 1;
  }

  u32(value) {
    this.view.setUint32(this.offset, value, false);
    this.offset += 4;
  }

  strBytes(bytes) {
    this.u32(bytes.length);
    this.bytes(bytes);
  }

  vecU32(values) {
    this.u32(values.length);
    values.forEach((value) => this.u32(value));
  }

  bytes(bytes) {
    new Uint8Array(this.buffer).set(bytes, this.offset);
    this.offset += bytes.length;
  }
}

class ByteReader {
  constructor(bytes) {
    this.bytesArray = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes);
    this.view = new DataView(this.bytesArray.buffer, this.bytesArray.byteOffset, this.bytesArray.byteLength);
    this.offset = 0;
  }

  remaining() {
    return this.bytesArray.byteLength - this.offset;
  }

  u8() {
    this.require(1);
    const value = this.view.getUint8(this.offset);
    this.offset += 1;
    return value;
  }

  u16() {
    this.require(2);
    const value = this.view.getUint16(this.offset, false);
    this.offset += 2;
    return value;
  }

  u32() {
    this.require(4);
    const value = this.view.getUint32(this.offset, false);
    this.offset += 4;
    return value;
  }

  str() {
    const length = this.u32();
    const bytes = this.bytes(length);
    return new TextDecoder().decode(bytes);
  }

  vecU8() {
    const length = this.u32();
    return Array.from({ length }, () => this.u8());
  }

  vecU16() {
    const length = this.u32();
    return Array.from({ length }, () => this.u16());
  }

  vecU32() {
    const length = this.u32();
    return Array.from({ length }, () => this.u32());
  }

  bytes(length) {
    this.require(length);
    const slice = this.bytesArray.slice(this.offset, this.offset + length);
    this.offset += length;
    return slice;
  }

  require(length) {
    if (this.offset + length > this.bytesArray.byteLength) {
      throw new Error("Response ended before it could be fully decoded.");
    }
  }
}

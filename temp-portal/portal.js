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
  serverUrl: document.querySelector("#server-url"),
  entryIds: document.querySelector("#entry-ids"),
  sendButton: document.querySelector("#send-button"),
  clearButton: document.querySelector("#clear-button"),
  status: document.querySelector("#status"),
  messageId: document.querySelector("#message-id"),
  recordCount: document.querySelector("#record-count"),
  payloadSize: document.querySelector("#payload-size"),
  entriesBody: document.querySelector("#entries-body"),
  rawOutput: document.querySelector("#raw-output"),
};

let nextMessageId = 1;

fields.form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const messageId = nextMessageId++;

  setStatus("loading", "Connecting");
  fields.sendButton.disabled = true;
  fields.messageId.textContent = String(messageId);

  try {
    const ids = parseIds(fields.entryIds.value);
    const response = await requestEntries(fields.serverUrl.value.trim(), ids, messageId);
    const decoded = decodeEntriesPayload(response.payload);
    renderEntries(decoded.entries);
    renderRaw(response, decoded);
    fields.recordCount.textContent = String(decoded.entries.length);
    fields.payloadSize.textContent = `${response.payloadSize} bytes`;
    setStatus("success", "Received");
  } catch (error) {
    setStatus("error", "Error");
    renderError(error);
  } finally {
    fields.sendButton.disabled = false;
  }
});

fields.clearButton.addEventListener("click", () => {
  renderEntries([]);
  fields.rawOutput.textContent = "Waiting for a request...";
  fields.messageId.textContent = "-";
  fields.recordCount.textContent = "0";
  fields.payloadSize.textContent = "0 bytes";
  setStatus("idle", "Idle");
});

function requestEntries(url, ids, messageId) {
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
        action: ACTION_TYPE.ACT_GET,
        id: messageId,
        payload: encodeVecU32(ids),
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

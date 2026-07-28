const ENCODE_VERSION = 1001;

const COM_TYPE = {
	REQ: 1,
	RESP: 2,
	STATUS: 3,
	PING: 4
};

const DOMAIN_TYPE = {
	ENTRY: 1,
	GROUPS: 2,
	CHARACTERS: 3,
	FILTERS: 4,
	USERS: 9,
	AUTH: 10
};

const ACTION_TYPE = {
	GET: 1,
	UPDATE: 2,
	CREATE: 3,
	DELETE: 4,
	SEARCH: 5,
	SORT: 6,
	APPLY: 7,
	FILTER: 8,
	LOGIN: 9,
	LOGOUT: 10
};

class ByteWriter {
	constructor() {
		this.bytes = [];
	}

	u8(value) {
		this.bytes.push(value & 0xff);
	}

	u32(value) {
		this.bytes.push((value >>> 24) & 0xff);
		this.bytes.push((value >>> 16) & 0xff);
		this.bytes.push((value >>> 8) & 0xff);
		this.bytes.push(value & 0xff);
	}

	str(value) {
		const encoded = new TextEncoder().encode(value);
		this.u32(encoded.length);
		this.bytes.push(...encoded);
	}

	finish() {
		return new Uint8Array(this.bytes);
	}
}

class ByteReader {
	constructor(bytes) {
		this.bytes = bytes;
		this.offset = 0;
		this.textDecoder = new TextDecoder();
	}

	ensure(size) {
		if (this.offset + size > this.bytes.length) {
			throw new Error("Response ended before the expected data was available.");
		}
	}

	u8() {
		this.ensure(1);
		return this.bytes[this.offset++];
	}

	bool() {
		return this.u8() !== 0;
	}

	u32() {
		this.ensure(4);

		const value =
			(this.bytes[this.offset] << 24) |
			(this.bytes[this.offset + 1] << 16) |
			(this.bytes[this.offset + 2] << 8) |
			this.bytes[this.offset + 3];

		this.offset += 4;
		return value >>> 0;
	}

	str() {
		const size = this.u32();
		this.ensure(size);

		const value = this.textDecoder.decode(this.bytes.slice(this.offset, this.offset + size));
		this.offset += size;
		return value;
	}

	remainingBytes() {
		const value = this.bytes.slice(this.offset);
		this.offset = this.bytes.length;
		return value;
	}
}

let nextMessageId = 1;
function makeMessage(domain, action, payload) {
	const w = new ByteWriter();

	w.u32(ENCODE_VERSION);
	w.u8(COM_TYPE.REQ);
	w.u8(domain);
	w.u8(action);
	w.u32(nextMessageId++);
	w.u32(payload.length);

	for (const byte of payload) {
		w.u8(byte);
	}

	return w.finish();
}

function readMessage(bytes) {
	const r = new ByteReader(bytes);
	const message = {
		encodeVersion: r.u32(),
		comType: r.u8(),
		domain: r.u8(),
		action: r.u8(),
		id: r.u32(),
		payloadSize: r.u32(),
		payload: null
	};

	r.ensure(message.payloadSize);
	message.payload = bytes.slice(r.offset, r.offset + message.payloadSize);

	return message;
}

function readAuthResult(payload) {
	const r = new ByteReader(payload);
	const success = r.bool();
	const hasUserId = r.bool();

	return {
		success,
		userId: hasUserId ? r.u32() : null,
		role: r.u8(),
		error: r.str()
	};
}

function actionName(value) {
	return Object.keys(ACTION_TYPE).find((key) => ACTION_TYPE[key] === value) ?? `ACTION_${value}`;
}

function roleName(value) {
	switch (value) {
		case 0:
			return "Viewer";
		case 1:
			return "Editor";
		case 2:
			return "Admin";
		default:
			return `Role ${value}`;
	}
}

function setAuthStatus(message, type = "") {
	const log = document.querySelector("#auth-response-log");

	if (!log) {
		return;
	}

	log.classList.toggle("is-success", type === "success");
	log.classList.toggle("is-error", type === "error");
	log.textContent = message;
}

function handleAuthResponse(message) {
	const result = readAuthResult(message.payload);
	const label = actionName(message.action);

	console.log("Auth response", { action: label, ...result });

	if (!result.success) {
		setAuthStatus(result.error || `${label} failed.`, "error");
		return;
	}

	if (result.userId !== null) {
		setAuthStatus(`${label} succeeded. Signed in as user ${result.userId} (${roleName(result.role)}).`, "success");
		return;
	}

	setAuthStatus(`${label} succeeded.`, "success");
}

function handleServerMessage(bytes) {
	try {
		const message = readMessage(bytes);

		console.log("Decoded response", message);

		if (message.encodeVersion !== ENCODE_VERSION) {
			throw new Error(`Unsupported response version ${message.encodeVersion}.`);
		}

		if (message.domain === DOMAIN_TYPE.AUTH) {
			handleAuthResponse(message);
			return;
		}

		setAuthStatus(`Received ${message.payloadSize} bytes from domain ${message.domain}.`);
	}
	catch (error) {
		console.error(error);
		setAuthStatus(error.message, "error");
	}
}



const socket = new WebSocket("ws://127.0.0.1:8080");
socket.binaryType = "arraybuffer";

socket.addEventListener("open", () => {
	console.log("connected");
});

socket.addEventListener("message", (event) => {
	const bytes = new Uint8Array(event.data);
	console.log("response bytes", bytes);
	handleServerMessage(bytes);
});

function makePasswordLoginPayload(loginKey, password) {
	const w = new ByteWriter();

	w.u8(0); // LoginProvider::PASSWORD
	w.str(loginKey);
	w.str(password);

	return w.finish();
}

function makeCreateAccountPayload(loginKey, password, email, displayName) {
	const w = new ByteWriter();

	w.u8(0); // LoginProvider::PASSWORD
	w.str(loginKey);
	w.str(password);
	w.str(email);
	w.str(displayName);

	return w.finish();
}

function sendAuthMessage(action, payload) {
	if (socket.readyState !== WebSocket.OPEN) {
		console.error("WebSocket is not connected.");
		return false;
	}

	const message = makeMessage(DOMAIN_TYPE.AUTH, action, payload);
	socket.send(message);
	return true;
}


const loginForm = document.querySelector(".login-form");
const createAccountForm = document.querySelector(".create-account-form");

if (loginForm) {
	const accountInput = document.querySelector("#account-name-text-field");
	const passwordInput = document.querySelector("#password-text-field");
	const passwordSection = document.querySelector(".password");
	const footerButton = document.querySelector("#create-account-link");
	const footerButtonText = footerButton?.querySelector("span");
	const createDisplayNameInput = document.querySelector("#create-display-name-field");
	const createLoginKeyInput = document.querySelector("#create-login-key-field");
	const createEmailInput = document.querySelector("#create-email-field");
	const createPasswordInput = document.querySelector("#create-password-field");
	const backToLoginButton = document.querySelector("#back-to-login-link");

	function setCreateAccountMode(enabled) {
		loginForm.hidden = enabled;

		if (createAccountForm) {
			createAccountForm.hidden = !enabled;
		}

		window.setTimeout(() => {
			if (enabled) {
				createDisplayNameInput?.focus();
			} else {
				accountInput?.focus();
			}
		}, 0);
	}

	function setPasswordStep(enabled) {
		loginForm.classList.toggle("password-step", enabled);

		if (passwordSection) {
			passwordSection.setAttribute("aria-hidden", String(!enabled));
		}

		if (footerButtonText) {
			footerButtonText.textContent = enabled ? "Forgot Password?" : "Create Account";
		}

		if (enabled) {
			window.setTimeout(() => passwordInput?.focus(), 180);
		}
	}

	passwordInput?.addEventListener("keydown", (event) => {
		if (event.key !== "Enter") {
			return;
		}

		event.preventDefault();

		const loginKey = accountInput.value.trim();
		const password = passwordInput.value;

		if (!loginKey || !password) {
			return;
		}

		const payload = makePasswordLoginPayload(loginKey, password);

		console.log("Sending login request", { loginKey });
		sendAuthMessage(ACTION_TYPE.LOGIN, payload);
	});

	accountInput?.addEventListener("keydown", (event) => {
		if (event.key !== "Enter") {
			return;
		}

		event.preventDefault();

		if (accountInput.value.trim() === "") {
			accountInput.focus();
			return;
		}

		setPasswordStep(true);
	});

	loginForm.addEventListener("submit", (event) => {
		event.preventDefault();

		if (!loginForm.classList.contains("password-step")) {
			if (accountInput?.value.trim()) {
				setPasswordStep(true);
			}
			return;
		}

		const loginKey = accountInput.value.trim();
		const password = passwordInput.value;

		if (!loginKey || !password) {
			return;
		}

		const payload = makePasswordLoginPayload(loginKey, password);
		sendAuthMessage(ACTION_TYPE.LOGIN, payload);
	});

	footerButton?.addEventListener("click", () => {
		if (loginForm.classList.contains("password-step")) {
			console.log("Forgot password flow");
			return;
		}

		setCreateAccountMode(true);
	});

	backToLoginButton?.addEventListener("click", () => {
		setCreateAccountMode(false);
	});

	createAccountForm?.addEventListener("submit", (event) => {
		event.preventDefault();

		const displayName = createDisplayNameInput?.value.trim() ?? "";
		const loginKey = createLoginKeyInput?.value.trim() ?? "";
		const email = createEmailInput?.value.trim() ?? "";
		const password = createPasswordInput?.value ?? "";

		if (!displayName || !loginKey || !email || !password) {
			console.error("Create account form is missing required fields.");
			return;
		}

		const payload = makeCreateAccountPayload(loginKey, password, email, displayName);

		console.log("Sending create account request", { loginKey, email, displayName });
		sendAuthMessage(ACTION_TYPE.CREATE, payload);
	});
}

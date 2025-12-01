// Configuration
const SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const COMMAND_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
const DATA_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a9";

// State
let device, server, service, commandChar, dataChar;
let currentBank = 0;
let selectedUiIndex = 0;
const LOGICAL_MAP = [0, 1, 2, 3];

let currentConfigs = [
  { type: 0, midiType: 0, value: 60, channel: 1, velocity: 127, enabled: 1 },
  { type: 0, midiType: 0, value: 61, channel: 1, velocity: 127, enabled: 1 },
  { type: 0, midiType: 0, value: 62, channel: 1, velocity: 127, enabled: 1 },
  { type: 0, midiType: 0, value: 63, channel: 1, velocity: 127, enabled: 1 },
];

// DOM Elements
const connectBtn = document.getElementById("connectBtn");
const statusDot = document.getElementById("statusDot");
const statusText = document.getElementById("statusText");
const mainInterface = document.getElementById("mainInterface");
const debugLog = document.getElementById("debugLog");
const saveBtn = document.getElementById("saveBtn");
const bankBtns = document.querySelectorAll(".bank-btn");

function log(msg) {
  const time = new Date().toLocaleTimeString();
  debugLog.innerHTML += `<div>[${time}] ${msg}</div>`;
  debugLog.scrollTop = debugLog.scrollHeight;
  console.log(msg);
}

// Bluetooth Connection
connectBtn.addEventListener("click", async () => {
  try {
    log("Requesting Bluetooth Device...");
    device = await navigator.bluetooth.requestDevice({
      filters: [{ name: "MIDI Pedalboard Config" }],
      optionalServices: [SERVICE_UUID],
    });

    device.addEventListener("gattserverdisconnected", onDisconnected);

    log("Connecting to GATT Server...");
    server = await device.gatt.connect();

    log("Getting Service...");
    service = await server.getPrimaryService(SERVICE_UUID);

    log("Getting Characteristics...");
    commandChar = await service.getCharacteristic(COMMAND_CHAR_UUID);
    dataChar = await service.getCharacteristic(DATA_CHAR_UUID);

    // Subscribe to notifications
    await dataChar.startNotifications();
    dataChar.addEventListener(
      "characteristicvaluechanged",
      handleDataNotification
    );

    log("Connected!");
    statusDot.classList.add("connected");
    statusText.textContent = "Connected";
    connectBtn.textContent = "Disconnect";
    connectBtn.onclick = disconnect;
    mainInterface.classList.add("visible");

    // Initial Load
    await loadConfig();
  } catch (error) {
    log("Connection failed: " + error);
  }
});

function disconnect() {
  if (device && device.gatt.connected) {
    device.gatt.disconnect();
  }
}

function onDisconnected() {
  log("Device Disconnected");
  statusDot.classList.remove("connected");
  statusText.textContent = "Disconnected";
  connectBtn.textContent = "Connect Bluetooth";
  connectBtn.onclick = null;
  mainInterface.classList.remove("visible");
}

// Handle notifications from data characteristic
function handleDataNotification(event) {
  const data = new Uint8Array(event.target.value.buffer);
  log(`Received notification: ${data.length} bytes`);
  parseConfig(data);
}

// Load config by reading data characteristic
async function loadConfig() {
  try {
    log("Reading configuration...");
    const value = await dataChar.readValue();
    const data = new Uint8Array(value.buffer);
    parseConfig(data);
  } catch (error) {
    log("Read failed: " + error);
  }
}

function parseConfig(data) {
  log(`Parsing ${data.length} bytes`);

  if (data.length < 25) {
    log(`Error: Invalid data length: ${data.length} bytes (expected 25)`);
    return;
  }

  currentBank = data[0];
  updateBankUI(currentBank);
  log(`Current Bank: ${currentBank + 1}`);

  for (let uiIdx = 0; uiIdx < 4; uiIdx++) {
    const fwIdx = LOGICAL_MAP[uiIdx];
    const offset = 1 + fwIdx * 6;

    currentConfigs[uiIdx] = {
      type: data[offset + 0],
      midiType: data[offset + 1],
      value: data[offset + 2],
      channel: data[offset + 3],
      velocity: data[offset + 4],
      enabled: data[offset + 5],
    };
    updatePedalInfo(uiIdx);
  }

  loadForm(selectedUiIndex);
}

async function saveCurrentConfig() {
  try {
    const uiIdx = selectedUiIndex;
    const fwIdx = LOGICAL_MAP[uiIdx];
    const cfg = currentConfigs[uiIdx];

    cfg.type = parseInt(document.getElementById("btnType").value);
    cfg.midiType = parseInt(document.getElementById("midiType").value);
    cfg.value = parseInt(document.getElementById("midiValue").value);
    cfg.channel = parseInt(document.getElementById("midiChannel").value);
    cfg.velocity = parseInt(document.getElementById("midiVelocity").value);

    const cmd = new Uint8Array([
      1,
      fwIdx,
      cfg.type,
      cfg.midiType,
      cfg.value,
      cfg.channel,
      cfg.velocity,
    ]);

    log(`Saving Button ${uiIdx + 1}...`);
    await commandChar.writeValue(cmd);
    log("Saved! Waiting for update...");
  } catch (error) {
    log("Save failed: " + error);
  }
}

async function setBank(bankIndex) {
  try {
    const cmd = new Uint8Array([2, bankIndex, 0, 0, 0, 0, 0]);
    log(`Switching to Bank ${bankIndex + 1}...`);
    await commandChar.writeValue(cmd);
    log("Waiting for update...");
  } catch (error) {
    log("Bank switch failed: " + error);
  }
}

function updateBankUI(bank) {
  bankBtns.forEach((btn) => {
    btn.classList.toggle("active", parseInt(btn.dataset.bank) === bank);
  });
}

function updatePedalInfo(uiIdx) {
  const cfg = currentConfigs[uiIdx];
  const info = document.getElementById(`info-${uiIdx}`);
  let typeStr = "";
  if (cfg.midiType === 0) typeStr = "NOTE";
  else if (cfg.midiType === 1) typeStr = "CC";
  else if (cfg.midiType === 2) typeStr = "PC";
  info.textContent = `${typeStr} ${cfg.value}`;
}

function loadForm(uiIdx) {
  const cfg = currentConfigs[uiIdx];
  document.getElementById("currentEditId").textContent = uiIdx + 1;
  document.getElementById("btnType").value = cfg.type;
  document.getElementById("midiType").value = cfg.midiType;
  document.getElementById("midiValue").value = cfg.value;
  document.getElementById("midiChannel").value = cfg.channel;
  document.getElementById("midiVelocity").value = cfg.velocity || 127;
}

window.selectPedal = (uiIdx) => {
  selectedUiIndex = uiIdx;
  document.querySelectorAll(".pedal-btn").forEach((btn, idx) => {
    btn.classList.toggle("selected", idx === uiIdx);
  });
  loadForm(uiIdx);
};

saveBtn.addEventListener("click", saveCurrentConfig);

bankBtns.forEach((btn) => {
  btn.addEventListener("click", (e) => {
    const bank = parseInt(e.target.dataset.bank);
    setBank(bank);
  });
});

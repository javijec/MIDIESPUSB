// Configuration
const SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
// State
let device, server, service, characteristic;
let currentBank = 0;
let selectedUiIndex = 0; // 0-3 (Left to Right on screen)
// LOGICAL MAPPING: UI Index (0-3) -> Firmware Index (3-0)
// Because firmware inverts: Phys 1 -> Log 3. We want UI 1 -> Phys 1 -> Log 3.
const LOGICAL_MAP = [0, 1, 2, 3];
// Cache of current configs (4 buttons)
let currentConfigs = [
  { type: 0, midiType: 0, value: 60, channel: 1, enabled: 1 },
  { type: 0, midiType: 0, value: 61, channel: 1, enabled: 1 },
  { type: 0, midiType: 0, value: 62, channel: 1, enabled: 1 },
  { type: 0, midiType: 0, value: 63, channel: 1, enabled: 1 },
];
// DOM Elements
const connectBtn = document.getElementById("connectBtn");
const statusDot = document.getElementById("statusDot");
const statusText = document.getElementById("statusText");
const mainInterface = document.getElementById("mainInterface");
const debugLog = document.getElementById("debugLog");
const saveBtn = document.getElementById("saveBtn");
const bankBtns = document.querySelectorAll(".bank-btn");
// --- Logging ---
function log(msg) {
  const time = new Date().toLocaleTimeString();
  debugLog.innerHTML += `<div>[${time}] ${msg}</div>`;
  debugLog.scrollTop = debugLog.scrollHeight;
  console.log(msg);
}
// --- Bluetooth Connection ---
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
    log("Getting Characteristic...");
    characteristic = await service.getCharacteristic(CHAR_UUID);
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
  connectBtn.onclick = null; // Reset handler
  mainInterface.classList.remove("visible");
}
// --- Data Handling ---
async function loadConfig() {
  try {
    log("Reading configuration...");
    const value = await characteristic.readValue();
    const data = new Uint8Array(value.buffer);
    log(`Received ${data.length} bytes`);
    if (data.length < 21) {
      log("Error: Invalid data length");
      return;
    }
    // Byte 0 is Current Bank
    currentBank = data[0];
    updateBankUI(currentBank);
    log(`Current Bank: ${currentBank + 1}`);
    // Parse Configs
    // Data format: [Bank, Btn0_Type, Btn0_Midi, Btn0_Val, Btn0_Ch, Btn0_En, Btn1...]
    // But remember, Firmware sends logical indices 0, 1, 2, 3.
    // We need to map these to our UI indices.
    // UI 0 maps to Firmware Index 3.
    for (let uiIdx = 0; uiIdx < 4; uiIdx++) {
      const fwIdx = LOGICAL_MAP[uiIdx];
      const offset = 1 + fwIdx * 5;
      currentConfigs[uiIdx] = {
        type: data[offset + 0],
        midiType: data[offset + 1],
        value: data[offset + 2],
        channel: data[offset + 3],
        enabled: data[offset + 4],
      };
      updatePedalInfo(uiIdx);
    }
    // Update Form for selected pedal
    loadForm(selectedUiIndex);
  } catch (error) {
    log("Read failed: " + error);
  }
}
async function saveCurrentConfig() {
  try {
    const uiIdx = selectedUiIndex;
    const fwIdx = LOGICAL_MAP[uiIdx];
    const cfg = currentConfigs[uiIdx];
    // Update config object from form
    cfg.type = parseInt(document.getElementById("btnType").value);
    cfg.midiType = parseInt(document.getElementById("midiType").value);
    cfg.value = parseInt(document.getElementById("midiValue").value);
    cfg.channel = parseInt(document.getElementById("midiChannel").value);
    // Prepare Command
    // [CMD=1, INDEX, TYPE, MIDITYPE, VALUE, CHANNEL]
    const cmd = new Uint8Array([
      1,
      fwIdx,
      cfg.type,
      cfg.midiType,
      cfg.value,
      cfg.channel,
    ]);
    log(`Saving Button ${uiIdx + 1} (FW Index ${fwIdx})...`);
    await characteristic.writeValue(cmd);
    log("Saved successfully!");
    updatePedalInfo(uiIdx);
  } catch (error) {
    log("Save failed: " + error);
  }
}
async function setBank(bankIndex) {
  try {
    // [CMD=2, BANK_INDEX, 0, 0, 0, 0]
    const cmd = new Uint8Array([2, bankIndex, 0, 0, 0, 0]);
    log(`Switching to Bank ${bankIndex + 1}...`);
    await characteristic.writeValue(cmd);
    // Wait a bit for flash write then reload
    setTimeout(loadConfig, 500);
  } catch (error) {
    log("Bank switch failed: " + error);
  }
}
// --- UI Updates ---
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
}
// --- Event Listeners ---
window.selectPedal = (uiIdx) => {
  selectedUiIndex = uiIdx;
  // Update visual selection
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

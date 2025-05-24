from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from typing import List
from bleak import BleakScanner
from model import Device, WiFiConfig
from ble_connect import bluetooth_Connect

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_methods=["*"],
    allow_headers=["*"],
)

scanned_devices = []

LED_UUID = "12345678-5678-90ab-cdef-1234567890ab"
SSID_UUID = "22345678-5678-90ab-cdef-1234567890ab"
PW_UUID = "32345678-5678-90ab-cdef-1234567890ab"

@app.get("/scan", response_model=List[Device])
async def scan_devices():
    scanned_devices = []
    devices = await BleakScanner.discover()
    for d in devices:
        if d.name and d.name.startswith("ESP32"):
            scanned_devices.append(Device(address=str(d.address), name=d.name))
    print("Scanned devices:", scanned_devices)
    return scanned_devices

@app.post("/connect/{address}")
async def conneting_device(address: str):
    SERVICE_UUID = None
    uuid_dict = {LED_UUID: "ON"}
    await bluetooth_Connect(address, uuid_dict)
    return {"message": "Welcome to the BLE Scanner API!"}

@app.post("/setup_wifi")
async def setup_wifi(config: WiFiConfig):
    SERVICE_UUID = None
    uuid_dict = {SSID_UUID: config.ssid, PW_UUID: config.password, LED_UUID: "OFF"}
    print(uuid_dict)
    print("WiFi Config:", config)
    await bluetooth_Connect(config.device_address, uuid_dict)

    print(f"Setup Wi-Fi on {config.device_name} with SSID={config.ssid} and Password={config.password}")

    return {"message": f"Wi-Fi set up to device: {config.device_name}"}


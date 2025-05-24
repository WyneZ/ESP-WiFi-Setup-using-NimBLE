from pydantic import BaseModel

class Device(BaseModel):
    address: str
    name: str = "Unknown"
    ssid: str = None
    password: str = None
    service_uuid: str = None

class WiFiConfig(BaseModel):
    ssid: str
    password: str
    device_name: str
    device_address: str
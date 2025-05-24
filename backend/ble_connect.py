from bleak import BleakClient
from bleak.exc import BleakError


async def bluetooth_Connect(address: str, uuid_dict: dict):
    service_uuid = None
    async with BleakClient(address) as client:
        connected = client.is_connected
        if not connected:
            raise BleakError(f"Could not connect to device {address}")
        print("🔗 Connected!")
        services = client.services

        print("Discovered services:")
        for s in services:
            if service_uuid is None and s.uuid.startswith("00000000-0000-0000-0000-"):
                service_uuid = s.uuid
                print("Service UUID:", service_uuid)
        
        if service_uuid:
            for uuid, value in uuid_dict.items():
                print(f"📡 Reading from {uuid}")                    
                char_value = await client.read_gatt_char(uuid)

                if uuid.startswith("12345678"):
                    message = value
                    await client.write_gatt_char(uuid, message.encode())
                    print("Sent LED:", message)
                elif uuid.startswith("22345678"):
                    message = value
                    await client.write_gatt_char(uuid, message.encode())
                    print("Sent SSID:", message)
                else:
                    message = value
                    await client.write_gatt_char(uuid, message.encode())
                    print("Sent Password:", message)
        else:
            print("❌ Service UUID not found in device")

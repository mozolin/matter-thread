import websocket
import json

# Replace with your Home Assistant URL and Long-Lived Access Token
HA_URL = "ws://192.168.1.101:8123/api/websocket"
HA_TOKEN = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI0MTQ5YzlmMDllMTA0MTg4YjRkYWY4Nzc4YWQxZTdhMiIsImlhdCI6MTc2MTgwOTI2NSwiZXhwIjoyMDc3MTY5MjY1fQ.FUJ7NOXYP0BrU_6o3syIy9H4UFlJOfCvT7BO0307pmE"

def get_devices():
    """Connects to the Home Assistant WebSocket API and fetches the device registry."""
    ws = websocket.create_connection(HA_URL)
    
    # 1. Authenticate with the Home Assistant server
    auth_message = {
        "type": "auth",
        "access_token": HA_TOKEN
    }
    ws.send(json.dumps(auth_message))
    
    auth_result = json.loads(ws.recv())
    if auth_result.get("type") != "auth_ok":
        print("Authentication failed.")
        return []

    # 2. Request the device registry
    device_list_message = {
        "id": 1,
        "type": "config/device_registry/list"
    }
    ws.send(json.dumps(device_list_message))
    
    # Wait for the response
    while True:
        response = json.loads(ws.recv())
        if response.get("id") == 1 and response.get("type") == "result":
            ws.close()
            return response.get("result", [])
    
def filter_and_print_devices(devices):
    """Filters the device list for Matter and Thread integrations and prints the results."""
    matter_devices = [d for d in devices if d.get('via_device_id') is not None and "matter" in d.get('via_device_id')]
    thread_devices = [d for d in devices if d.get('via_device_id') is not None and "thread" in d.get('via_device_id')]

    print("Matter Devices:")
    for device in matter_devices:
        print(f"  - {device.get('name')} (ID: {device.get('id')})")
        
    print("\nThread Devices:")
    for device in thread_devices:
        print(f"  - {device.get('name')} (ID: {device.get('id')})")

if __name__ == "__main__":
    all_devices = get_devices()
    filter_and_print_devices(all_devices)

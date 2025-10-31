import requests
import json
import websocket
import logging

# --- Settings ---
HA_ADDRESS = "192.168.1.101:8123"
HA_URL = f"http://{HA_ADDRESS}"
HA_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI0MTQ5YzlmMDllMTA0MTg4YjRkYWY4Nzc4YWQxZTdhMiIsImlhdCI6MTc2MTgwOTI2NSwiZXhwIjoyMDc3MTY5MjY1fQ.FUJ7NOXYP0BrU_6o3syIy9H4UFlJOfCvT7BO0307pmE"

ws_url = f"ws://{HA_ADDRESS}/api/websocket"
rest_api_url = f"{HA_URL}/api/states/sensor.orphaned_matter_devices"

headers = {
    "Authorization": f"Bearer {HA_TOKEN}",
    "Content-Type": "application/json",
}

# --- WebSocket Logic ---

_LOGGER = logging.getLogger(__name__)

all_devices = []
all_entities = []

def on_message(ws, message):
    #-- WebSocket incoming message handler
    data = json.loads(message)
    _LOGGER.info(f"Received message: {data}\n")
    
    if data['type'] == 'auth_required':
        auth_msg = {"type": "auth", "access_token": HA_TOKEN}
        ws.send(json.dumps(auth_msg))
        
    elif data['type'] == 'auth_ok':
        _LOGGER.info("Authentication successful!\n")
        ws.send(json.dumps({"id": 1, "type": "config/device_registry/list"}))
        ws.send(json.dumps({"id": 2, "type": "config/entity_registry/list"}))

    elif data['type'] == 'result':
        if data.get('id') == 1:
            global all_devices
            all_devices = data['result']
            check_complete(ws)
        elif data.get('id') == 2:
            global all_entities
            all_entities = data['result']
            check_complete(ws)

def check_complete(ws):
    #-- Checks whether both lists have been received and starts processing
    if all_devices and all_entities:
        orphaned_devices = find_devices_without_entities()
        update_ha_sensor(orphaned_devices)
        ws.close()

def find_devices_without_entities():
    #-- Finds devices that have no associated entities
    device_ids_with_entities = {entity['device_id'] for entity in all_entities if entity.get('device_id')}
    all_device_ids = {device['id'] for device in all_devices}
    
    devices_without_entities_ids = all_device_ids - device_ids_with_entities
    
    return [
        device for device in all_devices
        if device['id'] in devices_without_entities_ids
    ]

def update_ha_sensor(orphaned_devices):
    #-- Updates the sensor status in Home Assistant
    matter_orphans = [
        d for d in orphaned_devices
        if any(identifier_pair[0] == 'matter' for identifier_pair in d.get('identifiers', []))
    ]
    
    device_info_list = []
    for device in matter_orphans:
        device_info_list.append({
            'name': device.get('name'),
            'id': device.get('id'),
            'manufacturer': device.get('manufacturer'),
            'model': device.get('model'),
        })

    payload = {
        "state": len(device_info_list),
        "attributes": {
            "devices": device_info_list
        }
    }
    
    response = requests.post(rest_api_url, headers=headers, data=json.dumps(payload))
    _LOGGER.info(f"Response from HA: {response.status_code}\n")
    if response.status_code != 200:
        _LOGGER.error(f"Error updating sensor: {response.text}\n")

def on_error(ws, error):
    _LOGGER.error(f"WebSocket error: {error}\n")

def on_close(ws, close_status_code, close_msg):
    _LOGGER.info("WebSocket connection closed\n")

def on_open(ws):
    _LOGGER.info("WebSocket connection open\n")

if __name__ == "__main__":
    _LOGGER.info("Running script to search for orphaned Matter devices...\n")
    ws = websocket.WebSocketApp(ws_url,
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.run_forever()

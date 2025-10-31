import requests
import json

HA_URL = "http://192.168.1.101:8123"
#-- name: mozolin
ACCESS_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI0MTQ5YzlmMDllMTA0MTg4YjRkYWY4Nzc4YWQxZTdhMiIsImlhdCI6MTc2MTgwOTI2NSwiZXhwIjoyMDc3MTY5MjY1fQ.FUJ7NOXYP0BrU_6o3syIy9H4UFlJOfCvT7BO0307pmE"

headers = {
    "Authorization": f"Bearer {ACCESS_TOKEN}",
    "Content-Type": "application/json",
}

def get_devices():
    response = requests.get(f"{HA_URL}/api/config/device_registry/list", headers=headers)
    response.raise_for_status()
    return response.json()

def get_entities():
    response = requests.get(f"{HA_URL}/api/config/entity_registry/list", headers=headers)
    response.raise_for_status()
    return response.json()

devices = get_devices()
entities = get_entities()

matter_thread_devices = []
for device in devices:
    # Example: Filter based on known Matter/Thread related attributes
    if "matter" in device.get("integrations", []) or "thread" in device.get("integrations", []):
        matter_thread_devices.append(device)
    # You might also look at 'manufacturer', 'model', or other specific attributes

matter_thread_entities = []
for entity in entities:
    # Example: Filter based on entity_id or device_id linked to Matter/Thread devices
    if entity.get("platform") in ["matter", "thread"]: # or other specific platform identifiers
        matter_thread_entities.append(entity)

print("Matter and Thread Devices:")
for device in matter_thread_devices:
    print(f"- Name: {device.get('name')}, Manufacturer: {device.get('manufacturer')}, Model: {device.get('model')}")

print("\nMatter and Thread Entities:")
for entity in matter_thread_entities:
    print(f"- Entity ID: {entity.get('entity_id')}, Platform: {entity.get('platform')}")

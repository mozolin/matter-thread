#include <map>
#include <string>

const std::map<uint32_t, std::string> ENDPOINT_NAMES = {
    {0x000A, "Door Lock"},
    {0x000B, "Door Lock Controller"},
    {0x000E, "Aggregator"},
    {0x000F, "Generic Switch"},
    {0x0011, "Power Source"},
    {0x0012, "OTA Requestor"},
    {0x0013, "Bridged Node"},
    {0x0014, "OTA Provider"},
    {0x0015, "Contact Sensor"},
    {0x0016, "Root Node"},
    {0x0017, "Solar Power"},
    {0x0018, "Battery Storage"},
    {0x0019, "Secondary Network Interface"},
    {0x0020, "Streaming Audio Player"},
    {0x0021, "Casting Audio Player"},
    {0x0022, "Speaker"},
    {0x0023, "Casting Video Player"},
    {0x0024, "Content App"},
    {0x0028, "Basic Video Player"},
    {0x0029, "Casting Video Client"},
    {0x002B, "Fan"},
    {0x002C, "Air Quality Sensor"},
    {0x002D, "Air Purifier"},
    {0x0041, "Water Freeze Detector"},
    {0x0042, "Water Valve"},
    {0x0043, "Water Leak Detector"},
    {0x0044, "Rain Sensor"},
    {0x0070, "Refrigerator"},
    {0x0072, "Room Air Conditioner"},
    {0x0073, "Laundry Washer"},
    {0x0074, "Robotic Vacuum Cleaner"},
    {0x0075, "Dishwasher"},
    {0x0076, "Smoke CO Alarm"},
    {0x0077, "Cook Surface"},
    {0x0078, "Cooktop"},
    {0x0079, "Microwave Oven"},
    {0x007A, "Extractor Hood"},
    {0x007B, "Oven"},
    {0x007C, "Laundry Dryer"},
    {0x0090, "Network Infrastructure Manager"},
    {0x0100, "On/Off Light"},
    {0x0101, "Dimmable Light"},
    {0x0104, "Dimmer Switch"},
    {0x0105, "Color Dimmer Switch"},
    {0x0106, "Light Sensor"},
    {0x0107, "Occupancy Sensor"},
    {0x010A, "On/Off Plug-in Unit"},
    {0x010B, "Dimmable Plug-in Unit"},
    {0x010C, "Color Temperature Light"},
    {0x010D, "Extended Color Light"},
    {0x010F, "Mounted On/Off Control"},
    {0x0110, "Mounted Dimmable Load Control"},
    {0x0130, "Joint Fabric Administrator"},
    {0x0141, "Audio Doorbell"},
    {0x0142, "Camera"},
    {0x0146, "Chime"},
    {0x0147, "Camera Controller"},
    {0x0148, "Doorbell"},
    {0x0202, "Window Covering"},
    {0x0203, "Window Covering Controller"},
    {0x0230, "Closure"},
    {0x0231, "Closure Panel"},
    {0x023E, "Closure Controller"},
    {0x0301, "Thermostat"},
    {0x0302, "Temperature Sensor"},
    {0x0303, "Pump"},
    {0x0304, "Pump Controller"},
    {0x0305, "Pressure Sensor"},
    {0x0306, "Flow Sensor"},
    {0x0307, "Humidity Sensor"},
    {0x0309, "Heat Pump"},
    {0x050C, "Energy EVSE"},
    {0x050D, "Device Energy Management"},
    {0x050F, "Water Heater"},
    {0x0510, "Electrical Sensor"},
    {0x0511, "Electrical Utility Meter"},
    {0x0513, "Electrical Energy Tariff"},
    {0x0514, "Electrical Meter"},
    {0x0840, "Control Bridge"},
    {0x0850, "On/Off Sensor"},
};


/*
std::string get_endpoint_name(uint16_t endpoint_id)
{
    switch (endpoint_id) {
        case 0x000A: return "Door Lock";
        case 0x000B: return "Door Lock Controller";
        case 0x000E: return "Aggregator";
        case 0x0013: return "Bridged Node";
        case 0x0015: return "Contact Sensor";
        case 0x0017: return "Solar Power";
        case 0x0018: return "Battery Storage";
        case 0x0022: return "Speaker";
        case 0x0023: return "Casting Video Player";
        case 0x0024: return "Content App";
        case 0x0028: return "Basic Video Player";
        case 0x0029: return "Casting Video Client";
        case 0x002B: return "Fan";
        case 0x002C: return "Air Quality Sensor";
        case 0x002D: return "Air Purifier";
        case 0x0041: return "Water Freeze Detector";
        case 0x0042: return "Water Valve";
        case 0x0043: return "Water Leak Detector";
        case 0x0044: return "Rain Sensor";
        case 0x0070: return "Refrigerator";
        case 0x0072: return "Room Air Conditioner";
        case 0x0073: return "Laundry Washer";
        case 0x0074: return "Robotic Vacuum Cleaner";
        case 0x0075: return "Dishwasher";
        case 0x0076: return "Smoke CO Alarm";
        case 0x0077: return "Cook Surface";
        case 0x0078: return "Cooktop";
        case 0x0079: return "Microwave Oven";
        case 0x007A: return "Extractor Hood";
        case 0x007B: return "Oven";
        case 0x007C: return "Laundry Dryer";
        case 0x0100: return "On/Off Light";
        case 0x0101: return "Dimmable Light";
        case 0x0104: return "Dimmer Switch";
        case 0x0105: return "Color Dimmer Switch";
        case 0x010A: return "On/Off Plug-in Unit";
        case 0x010B: return "Dimmable Plug-in Unit";
        case 0x010C: return "Color Temperature Light";
        case 0x010D: return "Extended Color Light";
        case 0x010F: return "Mounted On/Off Control";
        case 0x0110: return "Mounted Dimmable Load Control";
        case 0x0141: return "Audio Doorbell";
        case 0x0142: return "Camera";
        case 0x0146: return "Chime";
        case 0x0147: return "Camera Controller";
        case 0x0148: return "Doorbell";
        case 0x0202: return "Window Covering";
        case 0x0203: return "Window Covering Controller";
        case 0x0230: return "Closure";
        case 0x0231: return "Closure Panel";
        case 0x023E: return "Closure Controller";
        case 0x0301: return "Thermostat";
        case 0x0302: return "Temperature Sensor";
        case 0x0303: return "Pump";
        case 0x0304: return "Pump Controller";
        case 0x0305: return "Pressure Sensor";
        case 0x0306: return "Flow Sensor";
        case 0x0307: return "Humidity Sensor";
        case 0x0309: return "Heat Pump";
        case 0x050C: return "Energy EVSE";
        case 0x050D: return "Device Energy Management";
        case 0x050F: return "Water Heater";
        case 0x0510: return "Electrical Sensor";
        case 0x0511: return "Electrical Utility Meter";
        case 0x0513: return "Electrical Energy Tariff";
        case 0x0514: return "Electrical Meter";
        case 0x0840: return "Control Bridge";
        case 0x0850: return "On/Off Sensor";
        default: return "Unknown";
    }
}
*/

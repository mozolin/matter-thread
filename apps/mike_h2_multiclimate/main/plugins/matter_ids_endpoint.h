#include <map>
#include <string>

/*
const std::map<uint32_t, std::string> ENDPOINT_NAMES = {
    {0x000A, "DoorLock"},
    {0x000B, "DoorLockController"},
    {0x000E, "Aggregator"},
    {0x0013, "BridgedNode"},
    {0x0015, "ContactSensor"},
    {0x0017, "SolarPower"},
    {0x0018, "BatteryStorage"},
    {0x0022, "Speaker"},
    {0x0023, "CastingVideoPlayer"},
    {0x0024, "ContentApp"},
    {0x0028, "BasicVideoPlayer"},
    {0x0029, "CastingVideoClient"},
    {0x002B, "Fan"},
    {0x002C, "AirQualitySensor"},
    {0x002D, "AirPurifier"},
    {0x0041, "WaterFreezeDetector"},
    {0x0042, "WaterValve"},
    {0x0043, "WaterLeakDetector"},
    {0x0044, "RainSensor"},
    {0x0070, "Refrigerator"},
    {0x0072, "RoomAirConditioner"},
    {0x0073, "LaundryWasher"},
    {0x0074, "RoboticVacuumCleaner"},
    {0x0075, "Dishwasher"},
    {0x0076, "SmokeCOAlarm"},
    {0x0077, "CookSurface"},
    {0x0078, "Cooktop"},
    {0x0079, "MicrowaveOven"},
    {0x007A, "ExtractorHood"},
    {0x007B, "Oven"},
    {0x007C, "LaundryDryer"},
    {0x0100, "OnOffLight"},
    {0x0101, "DimmableLight"},
    {0x0104, "DimmerSwitch"},
    {0x0105, "ColorDimmerSwitch"},
    {0x010A, "OnOffPluginUnit"},
    {0x010B, "DimmablePluginUnit"},
    {0x010C, "ColorTemperatureLight"},
    {0x010D, "ExtendedColorLight"},
    {0x010F, "MountedOnOffControl"},
    {0x0110, "MountedDimmableLoadControl"},
    {0x0141, "AudioDoorbell"},
    {0x0142, "Camera"},
    {0x0146, "Chime"},
    {0x0147, "Camera Controller"},
    {0x0148, "Doorbell"},
    {0x0202, "WindowCovering"},
    {0x0203, "WindowCoveringController"},
    {0x0230, "Closure"},
    {0x0231, "ClosurePanel"},
    {0x023E, "ClosureController"},
    {0x0301, "Thermostat"},
    {0x0302, "TemperatureSensor"},
    {0x0303, "Pump"},
    {0x0304, "PumpController"},
    {0x0305, "PressureSensor"},
    {0x0306, "FlowSensor"},
    {0x0307, "HumiditySensor"},
    {0x0309, "HeatPump"},
    {0x050C, "EnergyEVSE"},
    {0x050D, "DeviceEnergyManagement"},
    {0x050F, "WaterHeater"},
    {0x0510, "ElectricalSensor"},
    {0x0511, "ElectricalUtilityMeter"},
    {0x0513, "ElectricalEnergyTariff"},
    {0x0514, "ElectricalMeter"},
    {0x0840, "ControlBridge"},
    {0x0850, "OnOffSensor"},
};
*/

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

// matter_ids_decoder.cpp
#include "matter_ids_decoder.h"

const std::map<uint32_t, std::string> CLUSTER_NAMES = {
    {0x0003, "Identify"},
    {0x0004, "Groups"},
    {0x0005, "Scenes"},
    {0x0006, "On/Off"},
    {0x0008, "Level Control"},
    {0x001D, "Descriptor"},
    {0x001F, "Access Control"},
    {0x0028, "Basic Information"},
    {0x0029, "OTA Software Update Provider"},
    {0x002A, "OTA Software Update Requestor"},
    {0x0030, "General Commissioning"},
    {0x0031, "Network Commissioning"},
    {0x0033, "General Diagnostics"},
    {0x003C, "Administrator Commissioning"},
    {0x003E, "Operational Credentials"},
    {0x003F, "Group Key Management"},
    {0xFC00, "Custom: Chip Temperature"},
    {0xFC01, "Custom: Uptime"},
    // Добавьте другие стандартные кластеры по мере необходимости
};

const std::map<uint32_t, std::string> ATTRIBUTE_NAMES = {
    // Общие атрибуты
    {0x0000, "AttributeList"},
    {0x0001, "FeatureMap"},
    {0x0002, "ClusterRevision"},
    
    // Атрибуты Identify кластера
    {0x0000, "IdentifyTime"},
    
    // Атрибуты On/Off кластера
    {0x0000, "OnOff"},
    
    // Наши кастомные атрибуты
    {0x0000, "TemperatureValue"}, // Для кластера 0xFC00
    {0x0000, "UptimeSeconds"},   // Для кластера 0xFC01
    // Добавьте другие стандартные атрибуты по мере необходимости
};

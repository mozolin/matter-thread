#include <map>
#include <string>

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

#pragma once

using namespace esp_matter;

extern std::string get_cluster_name(uint32_t cluster_id);

extern std::string get_attribute_name(uint32_t cluster_id, uint32_t attribute_id);

extern std::string get_attribute_flags_string(uint16_t flags);

extern std::string get_attribute_type_name(esp_matter_val_type_t type);

extern std::string get_attribute_value(esp_matter_attr_val_t val);

extern void log_device_structure(node_t *node);


//-- ПРОВЕРКА СУЩЕСТВОВАНИЯ АТРИБУТА
//-- 1. Использование API esp_matter::attribute::get()
bool is_attribute_present(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id);
//-- 2. Проверка через ZAP-generated код
bool check_attribute_existence(uint32_t cluster_id, uint32_t attribute_id);
//-- 3. Получение списка всех атрибутов endpoint
void print_all_attributes(uint16_t endpoint_id);
//-- 4. Проверка через Matter API
//bool matter_attribute_exists(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id);
//-- 5. Комплексная проверка с валидацией типа
bool validate_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_val_type_t expected_type);

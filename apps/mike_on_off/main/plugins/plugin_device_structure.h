#pragma once

using namespace esp_matter;

extern std::string get_cluster_name(uint32_t cluster_id);

extern std::string get_attribute_name(uint32_t cluster_id, uint32_t attribute_id);

extern std::string get_attribute_flags_string(uint16_t flags);

extern std::string get_attribute_type_name(esp_matter_val_type_t type);

extern std::string get_attribute_value(esp_matter_attr_val_t val);

extern void log_device_structure(node_t *node, uint16_t parent_endpoint_id=0, int level=0);

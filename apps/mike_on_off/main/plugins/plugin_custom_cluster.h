#pragma once

#if ADD_CUSTOM_CLUSTERS

extern bool create_custom_cluster(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);

extern bool update_custom_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);

extern void force_attribute_reporting(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id);

extern void emulate_matter_cli();

#endif

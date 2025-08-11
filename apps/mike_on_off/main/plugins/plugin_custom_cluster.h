#pragma once

#if ADD_CUSTOM_CLUSTERS

extern bool create_custom_cluster(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);

extern bool update_custom_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);

extern void add_custom_cluster_safely(uint16_t endpoint_id);

extern CHIP_ERROR register_custom_endpoint(uint16_t endpoint_id, EmberAfEndpointType *new_endpoint);

extern void remove_custom_cluster(uint16_t endpoint_id);

#endif

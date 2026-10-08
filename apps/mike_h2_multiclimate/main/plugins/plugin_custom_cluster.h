#pragma once

#if ADD_CUSTOM_CLUSTERS

extern bool create_custom_cluster(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);
extern bool create_custom_cluster2(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);

extern bool update_custom_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val);

extern void add_custom_cluster_safely(uint16_t endpoint_id);

//extern void create_custom_cluster_safely(uint16_t endpoint_id);

extern CHIP_ERROR register_custom_endpoint(uint16_t endpoint_id, EmberAfEndpointType *new_endpoint);

extern void remove_custom_cluster(uint16_t endpoint_id);

extern uint32_t get_cluster_revision(chip::EndpointId endpoint, chip::ClusterId cluster);

extern void check_cluster_revisions();

extern void check_cluster_revision2();

#endif

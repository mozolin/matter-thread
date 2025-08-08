
#include <app_priv.h>

#if ADD_CUSTOM_CLUSTERS

#include "plugin_custom_cluster.h"

#include <map>
#include <string>
#include <inttypes.h>
#include "esp_matter.h"
#include "esp_matter_core.h"
#include "esp_matter_attribute.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "matter_ids_decoder.h"


using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

bool is_attribute_exists(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id)
{
  attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);
  return !attribute ? false : true;
}

bool create_custom_cluster(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val)
{
  //-- Get Matter node
  node_t *node = node::get();
  if(!node) {
  	ESP_LOGE(TAG_MIKE_APP, "Matter node not initialized");
  	return false;
	}
	//-- Get Endpoint
  endpoint_t *endpoint = endpoint::get(node, endpoint_id);
  if(!endpoint) {
  	ESP_LOGE(TAG_MIKE_APP, "Endpoint %d not found", endpoint_id);
  	return false;
	}

  //-- Create a custom cluster
  cluster_t *cluster = cluster::create(
  	endpoint,
  	cluster_id,
  	MATTER_CLUSTER_FLAG_SERVER
  );
  if(!cluster) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to create cluster");
  	return false;
	} else {
    //-- Check if attribute exists
    if(is_attribute_exists(endpoint_id, cluster_id, attribute_id)) {
    	ESP_LOGE(TAG_MIKE_APP, "Attribute already exists");
    	return false;
    }
    //-- Create an attribute
    esp_matter::attribute::create(
      cluster, 
      attribute_id, 
      //ATTRIBUTE_FLAG_NONE,
      MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NONVOLATILE,
      val
    );
    //-- Get a pointer to the attribute
    esp_matter::attribute_t *attribute = esp_matter::attribute::get(cluster, attribute_id);
    if(!attribute) {
  		ESP_LOGE(TAG_MIKE_APP, "Failed to create attribute");
  		return false;
		}

  }

  //-- Get a pointer to the cluster
  cluster_t *cluster_new = cluster::get(endpoint_id, cluster_id);
  if(!cluster_new) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to get cluster");
  	return false;
  } else {
  	ESP_LOGI(TAG_MIKE_APP, "~~~ Custom cluster 0x%08" PRIX32 " initialized at endpoint 0x%04" PRIX16, cluster_id, endpoint_id);
  }

  return true;
}

//-- Set a callback function if there is no other
static esp_err_t attr_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  ESP_LOGE(TAG_MIKE_APP, "~~~ attr_update_cb():type=%d,ep=%d,cluster=%d,attr=%d", (int)type, (int)endpoint_id, (int)cluster_id, (int)attribute_id);
  return ESP_OK;
}

//-- Update custom attribute
bool update_custom_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val)
{
  esp_err_t err = ESP_OK;
  bool is_read = true;

  //-- Set a callback function if there is no other
  attribute::set_callback(attr_update_cb);

  //-- Get a pointer to the Matter node
  node_t *node = node::get();
  if(!node) {
  	ESP_LOGE(TAG_MIKE_APP, "Matter node not initialized");
  	return false;
	}
	//-- Get a pointer to the Endpoint
  endpoint_t *endpoint = endpoint::get(node, endpoint_id);
  if(!endpoint) {
  	ESP_LOGE(TAG_MIKE_APP, "Endpoint %d not found", endpoint_id);
  	return false;
	}
  //-- Get a pointer to the cluster
  cluster_t *cluster = cluster::get(endpoint_id, cluster_id);
  if(!cluster) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to get cluster");
  	return false;
  }
  //-- Get a pointer to the attribute
  attribute_t *attribute = attribute::get(cluster, attribute_id);
  if(!attribute) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to get attribute");
  	return false;
	}
	//-- Get existing attribute value
	esp_matter_attr_val_t val_old;
	if(attribute::get_val(attribute, &val_old) == ESP_OK) {
  	const char* buf = get_attribute_value(val_old).c_str();
  	ESP_LOGI(TAG_MIKE_APP, "~~~ Get value of attribute: successfully => %s", buf);
  } else {
  	ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to get value of attribute");
  	return false;
  }

  //-- Print READ value of attribute
  is_read = true;
  attribute::val_print(endpoint_id, cluster_id, attribute_id, &val_old, is_read);

  //-- Try to update the attribute
  err = attribute::update(
    endpoint_id,
    cluster_id,
    attribute_id,
    &val
  );
  if(err != ESP_OK) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to update attribute");
  	return false;
  }

  //-- Try to report the attribute
  err = attribute::report(
  	endpoint_id,
  	cluster_id,
  	attribute_id,
  	&val
  );
  if(err != ESP_OK) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to report attribute");
  	return false;
  }

  //-- Print WRITE value of attribute
  is_read = false;
  attribute::val_print(endpoint_id, cluster_id, attribute_id, &val, is_read);

  return true;
}

#include <app-common/zap-generated/endpoint_config.h>
#include <app/util/attribute-storage.h>

void check_cluster_inclusion()
{
  const EmberAfEndpointType *endpoint_type = emberAfFindEndpointType(1);
  if(endpoint_type) {
    for(size_t i = 0; i < endpoint_type->clusterCount; i++) {
      ChipLogProgress(Zcl, "Cluster 0x%04X", endpoint_type->cluster[i].clusterId);
    }
  }
}

void manual_cluster_registration(uint16_t endpoint_id, uint32_t cluster_id)
{
  // 1. Определяем кастомный кластер
  //constexpr chip::ClusterId kCustomClusterId = 0xFC00;
  constexpr chip::ClusterId kCustomClusterId = cluster_id;
  
  // 2. Добавляем в список кластеров endpoint
  EmberAfCluster cluster = {
    .clusterId = kCustomClusterId,
    .attributes = nullptr, // Указываем свои атрибуты
    .attributeCount = 0,
    .clusterSize = 0
  };
  
  // 3. Регистрируем вручную (пример для endpoint 1)
  //emberAfEndpointEnableDisable(1, true, &cluster, 1);
  emberAfEndpointEnableDisable(endpoint_id, true, &cluster, 1);
}

#endif

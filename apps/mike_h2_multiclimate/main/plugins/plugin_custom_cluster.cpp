
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

  //add_custom_cluster_safely(endpoint_id);

  return true;
}

bool create_custom_cluster2(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val)
{
  //-- Get Matter node
  node_t *node = node::get();
  if(!node) {
  	ESP_LOGE(TAG_MIKE_APP, "Matter node not initialized");
  	return false;
	}
  	
  //!!! 0x09 MEANS CLUSTER #8, SO IT SHOULD NOT BE REMOVED !!!
  //remove_custom_cluster(0x09);

  remove_custom_cluster(0x0A);
  remove_custom_cluster(0x0B);
  remove_custom_cluster(0x0C);
  remove_custom_cluster(0x0D);
  remove_custom_cluster(0x0E);
  remove_custom_cluster(0x0F);
  
  endpoint_t *endpoint = endpoint::create(node, ENDPOINT_FLAG_NONE, nullptr);

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

		//-- Добавление атрибута ClusterRevision (0xFFFD)
    esp_matter::attribute::create(
      cluster, 
      0xFFFD, 
      MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NONVOLATILE,
      //-- revision = 1
      esp_matter_uint16(1)
    );
    //-- Get a pointer to the attribute
    esp_matter::attribute_t *attribute2 = esp_matter::attribute::get(cluster, 0xFFFD);
    if(!attribute2) {
  		ESP_LOGE(TAG_MIKE_APP, "Failed to create cluster revision attribute");
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

  //check_cluster_revision2();

  //add_custom_cluster_safely(endpoint_id);

  //create_custom_cluster_safely(9);

  //check_cluster_revisions();
  //check_cluster_revision2();

  return true;
}

/*
//-- Set a callback function if there is no other
static esp_err_t attr_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  ESP_LOGE(TAG_MIKE_APP, "~~~ attr_update_cb():type=%d,ep=%d,cluster=%d,attr=%d", (int)type, (int)endpoint_id, (int)cluster_id, (int)attribute_id);
  return ESP_OK;
}
*/

//-- Update custom attribute
bool update_custom_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val)
{
  esp_err_t err = ESP_OK;
  bool is_read = true;

  //-- Set a callback function if there is no other
  //attribute::set_callback(attr_update_cb);

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



#include <app/util/attribute-storage.h>
#include <app/util/af-types.h>
#include <app/util/endpoint-config-api.h>

EmberAfAttributeMetadata customAttributes[] =
{
  {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(1)),
    .attributeId   = 0xFFFD, // ClusterRevision
    .size          = 2,
    .attributeType = ZCL_INT16U_ATTRIBUTE_TYPE,
    .mask = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE,
  },
  {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
    .attributeId   = 0x0000,
    .size          = 4,
    .attributeType = ZCL_INT32U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE,
    
    /*
    .attributeId = 0x0000,
    .size = 2, // INT16 = 2 байта
    .attributeType = ZCL_INT16S_ATTRIBUTE_TYPE,
    .mask = MATTER_ATTRIBUTE_FLAG_WRITABLE, // или 0 для read-only
    */
  }
};

EmberAfCluster customCluster =
{
  .clusterId = 0xFC01,
  /*
  .attributes = nullptr, // Будет инициализировано позже
  .attributeCount = 0,
  */
  .attributes = customAttributes,
  .attributeCount = ARRAY_SIZE(customAttributes),
  .clusterSize = 0,
  .mask = MATTER_CLUSTER_FLAG_SERVER,
  .functions = nullptr,
  .acceptedCommandList = nullptr,
  .generatedCommandList = nullptr,
  .eventList = nullptr,
  .eventCount = 0,
};


#include <app/util/attribute-storage.h>
#include <app-common/zap-generated/ids/Clusters.h>


// Буфер для хранения ревизий кластеров
uint32_t cluster_revisions_buffer[5] = {0}; // Для 5 кластеров

void prepare_cluster_revisions()
{
  /*
  0x0000001D (Descriptor)
  0x00000003 (Identify)
  0x00000004 (Groups)
  0x00000006 (On/Off)
  0x0000FC00 (Custom: Chip Temperature)
  0x0000FC01 (Custom: Uptime)
  */

  // Для стандартных кластеров
  cluster_revisions_buffer[0] = get_cluster_revision(emberAfEndpointFromIndex(0), 0x0000001D);
  cluster_revisions_buffer[1] = get_cluster_revision(emberAfEndpointFromIndex(0), 0x00000003);
  cluster_revisions_buffer[2] = get_cluster_revision(emberAfEndpointFromIndex(0), 0x00000004);
  cluster_revisions_buffer[3] = get_cluster_revision(emberAfEndpointFromIndex(0), 0x00000006);

  // Для кастомного кластера (0xFC01)
  cluster_revisions_buffer[4] = 1; // Ревизия 1

}

#include <app/util/attribute-storage.h>
#include <app-common/zap-generated/attribute-type.h>

//-- emberAfReadAttribute
#include <app/util/attribute-table.h>

//-- EMBER_ZCL_STATUS_SUCCESS, etc
#include <protocols/interaction_model/StatusCode.h>
using namespace ::chip;
using namespace ::chip::Protocols::InteractionModel;


uint32_t get_cluster_revision(chip::EndpointId endpoint, chip::ClusterId cluster)
{
  // Стандартный путь для получения ревизии
  const EmberAfEndpointType *ep = emberAfFindEndpointType(endpoint);
  if(!ep) {
  	//const EmberAfCluster * emberAfFindClusterInType(const EmberAfEndpointType * endpointType, chip::ClusterId clusterId, EmberAfClusterMask mask, uint8_t * index = nullptr);
    const EmberAfCluster *cluster_ptr = emberAfFindClusterInType(ep, cluster, MATTER_CLUSTER_FLAG_SERVER);
    if(cluster_ptr && cluster_ptr->attributes) {
      for(size_t i = 0; i < cluster_ptr->attributeCount; i++) {
        if(cluster_ptr->attributes[i].attributeId == 0xFFFD) { // ClusterRevision attribute ID
          uint16_t revision;
          if(emberAfReadAttribute(endpoint, cluster, 0xFFFD, (uint8_t*)&revision, sizeof(revision)) == Status::Success) {
            return revision;
          }
        }
      }
    }
  }
  return 1; // Default revision
}

void add_custom_cluster_safely(uint16_t endpoint_id)
{
  // 1. Получаем текущий endpoint (без модификации)
  const EmberAfEndpointType *endpoint = emberAfFindEndpointType(endpoint_id);
  if(!endpoint) {
    ESP_LOGE(TAG_MIKE_APP, "Endpoint %d not found", endpoint_id);
    return;
  }

  // 2. Создаем копию структуры endpoint
  EmberAfEndpointType *new_endpoint = (EmberAfEndpointType*)chip::Platform::MemoryAlloc(sizeof(EmberAfEndpointType));
  memcpy(new_endpoint, endpoint, sizeof(EmberAfEndpointType));
  
  // 3. Создаем новый массив кластеров (+1 для нашего)
  size_t new_cluster_count = endpoint->clusterCount + 1;
  EmberAfCluster *new_clusters = (EmberAfCluster*)chip::Platform::MemoryAlloc(sizeof(EmberAfCluster) * new_cluster_count);
  
  // 4. Копируем существующие кластеры
  memcpy(new_clusters, endpoint->cluster, sizeof(EmberAfCluster) * endpoint->clusterCount);
  
  // 5. Инициализируем кастомный кластер
  //customCluster.attributes = customAttributes;
  //customCluster.attributeCount = ARRAY_SIZE(customAttributes);
  
  // 6. Добавляем наш кластер
  new_clusters[endpoint->clusterCount] = customCluster;
  ESP_LOGW(TAG_MIKE_APP, "new_clusters-1: %d (%d -> %d)", endpoint->clusterCount, new_cluster_count, sizeof(new_clusters));
  
  // 7. Обновляем структуру
  new_endpoint->cluster = new_clusters;
  new_endpoint->clusterCount = new_cluster_count;
  
  ESP_LOGW(TAG_MIKE_APP, "new_clusters-2: %d", new_endpoint->clusterCount);
  
  // 8. Регистрируем обновленный endpoint
  CHIP_ERROR err = CHIP_NO_ERROR;
  
  // 1. Подготовка параметров
  uint16_t index = 0; // Индекс в пуле динамических endpoint
  chip::EndpointId chipEndpointId = endpoint_id; // ID нашего endpoint (не 0!)
  
  // 2. Список ревизий кластеров
  prepare_cluster_revisions();
  chip::Span<uint32_t> revisions_span(cluster_revisions_buffer, new_endpoint->clusterCount);
  /*
  uint32_t revisions[] = {1}; // Для кастомного кластера
  chip::Span<uint32_t> revisions_span(revisions);
  */
  
  // 3. Device types (может быть пустым)
  chip::Span<const chip::app::DataModel::DeviceTypeEntry> device_types;
  
  // 4. Parent endpoint (0 если нет)
  chip::EndpointId chipParentEndpointId = 0;
  
  // 5. Регистрация endpoint
  //-- emberAfSetDynamicEndpoint(idx, epId, ep, dataVersionStorage, deviceTypeList);
  err = emberAfSetDynamicEndpoint(
    index,               //index              //-- uint16_t index
    chipEndpointId,      //mCurrentEndpointId //-- EndpointId id
    new_endpoint,        //ep                 //-- const EmberAfEndpointType * ep
    revisions_span,      //dataVersionStorage //-- const Span<DataVersion> & dataVersionStorage
    device_types,        //deviceTypeList     //-- Span<const EmberAfDeviceType> deviceTypeList
    chipParentEndpointId //---                //-- EndpointId parentEndpointId
  );

  //remove_custom_cluster(endpoint_id);

  if(err != CHIP_NO_ERROR) {
    if(err == CHIP_ERROR_NO_MEMORY) {
    	ESP_LOGE(TAG_MIKE_APP, "Failed to register endpoint: CHIP_ERROR_NO_MEMORY");
    }
    if(err == CHIP_ERROR_INVALID_ARGUMENT) {
    	ESP_LOGE(TAG_MIKE_APP, "Failed to register endpoint: CHIP_ERROR_INVALID_ARGUMENT");
    }
    if(err == CHIP_ERROR_ENDPOINT_EXISTS) {
    	ESP_LOGE(TAG_MIKE_APP, "Failed to register endpoint: CHIP_ERROR_ENDPOINT_EXISTS");
    }
  } else {
    ESP_LOGI(TAG_MIKE_APP, "Custom endpoint %d registered", endpoint_id);
  }

  ESP_LOGI(TAG_MIKE_APP, "Custom cluster 0x%08" PRIX32 " added to endpoint %d", 
          customCluster.clusterId, endpoint_id);

  chip::EndpointId ep = emberAfEndpointFromIndex(1);
  bool isServer = true;
  uint8_t num = emberAfClusterCount(ep, isServer);
	ESP_LOGW(TAG_MIKE_APP, "Clusters: %d", num);
}

/*
void create_custom_cluster_safely(uint16_t endpoint_id)
{
  ESP_LOGE("", "");
  ESP_LOGE(TAG_MIKE_APP, "##########################");
  ESP_LOGE(TAG_MIKE_APP, "#                        #");
  ESP_LOGE(TAG_MIKE_APP, "#   CUSTOM ENDPOINT: %d   #", endpoint_id);
  ESP_LOGE(TAG_MIKE_APP, "#                        #");
  ESP_LOGE(TAG_MIKE_APP, "##########################");
  ESP_LOGE("", "");
  
  EmberAfEndpointType endpoint = {
    .cluster = &customCluster,
    .clusterCount = 1,
    .endpointSize = 0
  };
  
  ESP_LOGW(TAG_MIKE_APP, "clusters: %d", endpoint->clusterCount);
  
  // 8. Регистрируем обновленный endpoint
  CHIP_ERROR err = CHIP_NO_ERROR;
  
  // 1. Подготовка параметров
  uint16_t index = 0; // Индекс в пуле динамических endpoint
  chip::EndpointId chipEndpointId = endpoint_id; // ID нашего endpoint (не 0!)
  
  // 2. Список ревизий кластеров
  prepare_cluster_revisions();
  chip::Span<uint32_t> revisions_span(cluster_revisions_buffer, endpoint->clusterCount);
  //uint32_t revisions[] = {1}; // Для кастомного кластера
  //chip::Span<uint32_t> revisions_span(revisions);
  
  // 3. Device types (может быть пустым)
  chip::Span<const chip::app::DataModel::DeviceTypeEntry> device_types;
  
  // 4. Parent endpoint (0 если нет)
  chip::EndpointId chipParentEndpointId = 0;
  
  // 5. Регистрация endpoint
  //-- emberAfSetDynamicEndpoint(idx, epId, ep, dataVersionStorage, deviceTypeList);
  err = emberAfSetDynamicEndpoint(
    index,               //index              //-- uint16_t index
    chipEndpointId,      //mCurrentEndpointId //-- EndpointId id
    endpoint,            //ep                 //-- const EmberAfEndpointType * ep
    revisions_span,      //dataVersionStorage //-- const Span<DataVersion> & dataVersionStorage
    device_types,        //deviceTypeList     //-- Span<const EmberAfDeviceType> deviceTypeList
    chipParentEndpointId //---                //-- EndpointId parentEndpointId
  );

  //remove_custom_cluster(endpoint_id);

  if(err != CHIP_NO_ERROR) {
    if(err == CHIP_ERROR_NO_MEMORY) {
    	ESP_LOGE(TAG_MIKE_APP, "Failed to register endpoint: CHIP_ERROR_NO_MEMORY");
    }
    if(err == CHIP_ERROR_INVALID_ARGUMENT) {
    	ESP_LOGE(TAG_MIKE_APP, "Failed to register endpoint: CHIP_ERROR_INVALID_ARGUMENT");
    }
    if(err == CHIP_ERROR_ENDPOINT_EXISTS) {
    	ESP_LOGE(TAG_MIKE_APP, "Failed to register endpoint: CHIP_ERROR_ENDPOINT_EXISTS");
    }
  } else {
    ESP_LOGI(TAG_MIKE_APP, "Custom endpoint %d registered", endpoint_id);
  }

  ESP_LOGI(TAG_MIKE_APP, "Custom cluster 0x%08" PRIX32 " added to endpoint %d", 
          customCluster.clusterId, endpoint_id);

  chip::EndpointId ep = emberAfEndpointFromIndex(1);
  bool isServer = true;
  uint8_t num = emberAfClusterCount(ep, isServer);
	ESP_LOGW(TAG_MIKE_APP, "Clusters: %d", num);
}
*/

void remove_custom_cluster(uint16_t endpoint_id)
{
  ESP_LOGW(TAG_MIKE_APP, "Remove EP %d", endpoint_id);
  emberAfClearDynamicEndpoint(endpoint_id - 1);
}

/*
void check_cluster_revisions()
{
  chip::EndpointId endpoint_id = 1;
  for(uint16_t i = 0; i < emberAfEndpointCount(); i++) {
    //const EmberAfEndpointType *ept = emberAfEndpointFromIndex(i);
    chip::EndpointId ept = emberAfEndpointFromIndex(i);
    if(ept->endpoint == endpoint_id) {
      for(uint8_t j = 0; j < ept->clusterCount; j++) {
        uint32_t rev = get_cluster_revision(endpoint_id, ept->cluster[j].clusterId);
        ESP_LOGI(TAG_MIKE_APP, "Cluster 0x%08" PRIX32 " revision: %d", ept->cluster[j].clusterId, (int)rev);
      }
    }
  }
}
*/

void check_cluster_revision2()
{
  uint16_t revision = 0;
  if(emberAfReadAttribute(1, 0xFC01, 0xFFFD, 
    (uint8_t*)&revision, sizeof(revision)) == Status::Success) {
    ESP_LOGW(TAG_MIKE_APP, "Cluster 0xFC01 revision: %d", revision);
  } else {
  	ESP_LOGE(TAG_MIKE_APP, "Cluster 0xFC01 revision not found!");
  }
}

/*
0x00 EMBER_ZCL_STATUS_SUCCESS: Command request was successful
0x01 EMBER_ZCL_STATUS_FAILURE: Command request failed - for example, a call to remove an entry from the group table returned an error
0x80 EMBER_ZCL_STATUS_MALFORMED_COMMAND: no RFData in the API frame; ZCL Payload appears truncated from what is expected
0x81 EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND: unexpected direction in the Frame Control Field of the ZCL Header; unexpected command identifier code value in the ZCL header
0x82 EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND: unexpected frametype in the Frame Control Field of the ZCL Header
0x84 EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND: unexpected manufacturer specific indication in the Frame Control Field of the ZCL Header
0x8b EMBER_ZCL_STATUS_NOT_FOUND: An attempt at Get Group Membership or Remove Group could not find a matching entry in the group table
*/

#endif

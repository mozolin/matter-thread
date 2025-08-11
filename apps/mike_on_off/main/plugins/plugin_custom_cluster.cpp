
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

  add_custom_cluster_safely(endpoint_id);

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



#include <app/util/attribute-storage.h>
#include <app/util/af-types.h>
#include <app/util/endpoint-config-api.h>

EmberAfAttributeMetadata customAttributes[] =
{
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

#include <lib/support/CodeUtils.h>
#include <app/util/basic-types.h>
#include <app/ConcreteClusterPath.h>


using namespace chip::app::DataModel;


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
  ESP_LOGW(TAG_MIKE_APP, "new_clusters-1: %d (%d)", endpoint->clusterCount, new_cluster_count);

  // 7. Обновляем структуру
  new_endpoint->cluster = new_clusters;
  new_endpoint->clusterCount = new_cluster_count;

  ESP_LOGW(TAG_MIKE_APP, "new_clusters-2: %d", new_endpoint->clusterCount);

  // 8. Регистрируем обновленный endpoint
  CHIP_ERROR err = CHIP_NO_ERROR;
  
  // 1. Подготовка параметров
  uint16_t index = 0; // Индекс в пуле динамических endpoint
  chip::EndpointId chipEndpointId = endpoint_id; // ID нашего endpoint (не 0!)
  
  // 2. Список кластеров (если нужно)
  //-- const chip::Span<chip::DataVersion> & dataVersionStorage
  chip::Span<uint32_t> cluster_revisions;

  //-- chip::DataVersion * emberAfDataVersionStorage(const chip::app::ConcreteClusterPath & aConcreteClusterPath);
  //-- chip::DataVersion * versionPtr = emberAfDataVersionStorage(ConcreteClusterPath(endpointId, cluster.clusterId));
  
  //chip::app::ConcreteClusterPath ConcreteClusterPath;
  
  //ConcreteClusterPath(EndpointId aEndpointId, ClusterId aClusterId)
  //chip::EndpointId aEndpointId = 1;
  //customCluster.clusterId aClusterId = 0xFC01;
  //chip::DataVersion * versionPtr = emberAfDataVersionStorage(ConcreteClusterPath(aEndpointId, customCluster.clusterId));
  
  /*
  cluster_revisions[0] = 1;
  cluster_revisions[1] = 1;
  cluster_revisions[2] = 1;
  cluster_revisions[3] = 1;
  cluster_revisions[4] = 1;
  */
  //std::vector<int> numbers = {1, 2, 3, 4, 5};
  
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
    cluster_revisions,   //dataVersionStorage //-- const Span<DataVersion> & dataVersionStorage
    device_types,        //deviceTypeList     //-- Span<const EmberAfDeviceType> deviceTypeList
    chipParentEndpointId //---                //-- EndpointId parentEndpointId
  );

  /*
  emberAfSetDynamicEndpoint(
  	uint16_t index,
  	EndpointId id,
  	const EmberAfEndpointType * ep,
    const Span<DataVersion> & dataVersionStorage,
    Span<const EmberAfDeviceType> deviceTypeList,
    EndpointId parentEndpointId
  )

  emberAfSetDynamicEndpoint(
  	index,
  	mCurrentEndpointId,
  	ep,
  	dataVersionStorage,
  	deviceTypeList
  );
  */
  
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

void remove_custom_cluster(uint16_t endpoint_id)
{
  emberAfClearDynamicEndpoint(endpoint_id - 1);
}

#endif

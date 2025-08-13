
#include <app_priv.h>

#include "plugin_device_structure.h"

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


std::string get_cluster_name(uint32_t cluster_id)
{
  auto it = CLUSTER_NAMES.find(cluster_id);
  if (it != CLUSTER_NAMES.end()) {
      return it->second;
  }
  return "Unknown";
}

std::string get_attribute_name(uint32_t cluster_id, uint32_t attribute_id)
{
  std::string str = "Unknown";

  // Общие атрибуты
  switch(attribute_id) {
  	case 0x0000:
  		str = "AttributeList";
  		break;
  	case 0x0001:
  		str = "FeatureMap";
  		break;
  	case 0x0002:
  		str = "ClusterRevision";
  		break;
  	case 0xFFFD:
  		str = "ClusterRevision";
  		break;
  }

  switch(cluster_id) {
    // Атрибуты Identify кластера
    case 0x0003:
    	switch(attribute_id) {
    		case 0x0000:
    			return "IdentifyTime";
    	}
    	break;
    // Атрибуты On/Off кластера
    case 0x0006:
    	switch(attribute_id) {
    		case 0x0000:
    			return "OnOff";
    	}
    	break;
    // Атрибуты кастомного 0xFC00 кластера
    case 0xFC00:
    	switch(attribute_id) {
    		case 0x0000:
    			return "TemperatureValue";
    	}
    	break;
    // Атрибуты кастомного 0xFC01 кластера
    case 0xFC01:
    	switch(attribute_id) {
    		case 0x0000:
    			return "UptimeSeconds";
    	}
    	break;
  }
  /*
  for(map<string,map<string,string> >::const_iterator ptr=ATTRIBUTE_NAMES.begin();ptr!=ATTRIBUTE_NAMES.end(); ptr++) {
    //cout << ptr->first << "\n";
    for( map<string,string>::const_iterator eptr=ptr->second.begin();eptr!=ptr->second.end(); eptr++){
    	//cout << eptr->first << " " << eptr->second << endl;
    	return eptr->second;
    }
	}
	*/
  
  /*
  auto cl = ATTRIBUTE_NAMES.find(cluster_id);
  if (cl != ATTRIBUTE_NAMES.end()) {
  	auto it = cl->second.find(attribute_id);
  	if (it != cl->second.end()) {
      return it->second;
  	}
  }
  */

  /*
  auto it = ATTRIBUTE_NAMES.find(attribute_id);
  if(it != ATTRIBUTE_NAMES.end()) {
    return it->second;
  }
  */
  return str;
}

std::string get_attribute_flags_string(uint16_t flags)
{
  // Проверяем, что flags содержат допустимые значения
  if(flags == 0) {
  	return "-";
  }
  
  std::string result;
  const struct {
    uint16_t flag;
    char symbol;
  } flag_map[] = {
    {ATTRIBUTE_FLAG_NONE,               'N'},
    {MATTER_ATTRIBUTE_FLAG_WRITABLE,    'W'},
    {MATTER_ATTRIBUTE_FLAG_NONVOLATILE, 'V'},
    {ATTRIBUTE_FLAG_NULLABLE,           'U'}
  };
  
  for(const auto &item : flag_map) {
    if(flags & item.flag) {
      result += item.symbol;
    }
  }
  
  return result.empty() ? "-" : result;
}

std::string get_attribute_type_name(esp_matter_val_type_t type)
{
  const std::map<esp_matter_val_type_t, std::string> TYPE_NAMES = {
    //{ESP_MATTER_VAL_TYPE_NULL,    "NULL"},
    {ESP_MATTER_VAL_TYPE_BOOLEAN, "BOOLEAN"},
    {ESP_MATTER_VAL_TYPE_INT8,    "INT8"},
    {ESP_MATTER_VAL_TYPE_UINT8,   "UINT8"},
    {ESP_MATTER_VAL_TYPE_INT16,   "INT16"},
    {ESP_MATTER_VAL_TYPE_UINT16,  "UINT16"},
    {ESP_MATTER_VAL_TYPE_INT32,   "INT32"},
    {ESP_MATTER_VAL_TYPE_UINT32,  "UINT32"},
    {ESP_MATTER_VAL_TYPE_INT64,   "INT64"},
    {ESP_MATTER_VAL_TYPE_UINT64,  "UINT64"},
    {ESP_MATTER_VAL_TYPE_FLOAT,   "FLOAT"},
    //{ESP_MATTER_VAL_TYPE_DOUBLE,  "DOUBLE"},
    {ESP_MATTER_VAL_TYPE_CHAR_STRING,  "STRING"},
    {ESP_MATTER_VAL_TYPE_ARRAY,   "ARRAY"},
    //{ESP_MATTER_VAL_TYPE_STRUCT,  "STRUCT"},
  };
  
  auto it = TYPE_NAMES.find(type);
  if(it != TYPE_NAMES.end()) {
    return it->second;
  }
  return "UNKNOWN";
}

std::string get_attribute_value(esp_matter_attr_val_t val)
{
	char buf[16];
	std::string type = get_attribute_type_name(val.type);
	std::string str = "UNKNOWN";
	
	if(type == "INT16") {
		snprintf(buf, sizeof(buf), "%d", val.val.i16);
		str.assign(buf);
	} else if(type == "UINT16") {
		snprintf(buf, sizeof(buf), "%d", val.val.u16);
		str.assign(buf);
	} else if(type == "INT32") {
		snprintf(buf, sizeof(buf), "%lu", val.val.i32);
		str.assign(buf);
	} else if(type == "UINT32") {
		snprintf(buf, sizeof(buf), "%lu", val.val.u32);
		str.assign(buf);
	}
	return str;

	/*
  //-- Boolean
  bool b;
  //-- Integer
  int i;
  //-- Float
  float f;
  //-- 8 bit signed integer
  int8_t i8;
  //-- 8 bit unsigned integer
  uint8_t u8;
  //-- 16 bit signed integer
  int16_t i16;
  //-- 16 bit unsigned integer
  uint16_t u16;
  //-- 32 bit signed integer
  int32_t i32;
  //-- 32 bit unsigned integer
  uint32_t u32;
  //-- 64 bit signed integer
  int64_t i64;
  //-- 64 bit unsigned integer
  uint64_t u64;
  //-- Array
  struct {
      //-- Buffer
      uint8_t *b;
      //-- Data size
      uint16_t s;
      //-- Data count
      uint16_t n;
      //-- Total size
      uint16_t t;
  } a;
  //-- Pointer
  void *p;
  */
}

//-- Default values for "parent_endpoint_id" and "level" are defined in the header file (as "extern")
void log_device_structure(node_t *node, uint16_t parent_endpoint_id, int level)
{
  if(!node) {
  	return;
  }

  std::vector<uint16_t> endpoint_ids;
  endpoint_t *endpoint = endpoint::get_first(node);
  while(endpoint) {
    endpoint_ids.push_back(endpoint::get_id(endpoint));
    endpoint = endpoint::get_next(endpoint);
  }

  for(uint16_t endpoint_id : endpoint_ids) {
    endpoint = endpoint::get(node, endpoint_id);
    if(!endpoint) {
    	continue;
    }
    
    #if SHOW_CUSTOM_ENDPOINT_ID_ONLY
    	//-- !! show the CUSTOM_ENDPOINT_ID endpoint ONLY !!
    	if(endpoint_id != CUSTOM_ENDPOINT_ID) {
    		continue;
    	}
    #endif

    std::string indent(level * 2, ' ');
    ESP_LOGE(TAG_EMPTY, "# %sEndpoint 0x%04" PRIX16, indent.c_str(), endpoint_id);

    //-- 3. Получение списка всех атрибутов endpoint
		ESP_LOGI(TAG_EMPTY, "");
		ESP_LOGI(TAG_EMPTY, "====================================");
		print_all_attributes(endpoint_id);
		ESP_LOGI(TAG_EMPTY, "====================================");
		ESP_LOGI(TAG_EMPTY, "");

    #if SHOW_DEVICE_LOG_CLUSTERS
      cluster_t *cluster = cluster::get_first(endpoint);
      while(cluster) {
        uint32_t cluster_id = cluster::get_id(cluster);
        std::string cluster_name = get_cluster_name(cluster_id);
        
        ESP_LOGW(TAG_EMPTY, "# %s  Cluster 0x%08" PRIX32 " (%s)", 
                indent.c_str(),
                cluster_id,
                cluster_name.c_str());
      
        #if SHOW_DEVICE_LOG_ATTRIBUTES
          attribute_t *attribute = attribute::get_first(cluster);
          while(attribute) {
            uint32_t attribute_id = attribute::get_id(attribute);

            
            //-- 1. Использование API esp_matter::attribute::get()
						if(!is_attribute_present(endpoint_id, cluster_id, attribute_id)) {
							ESP_LOGE("APP", "Attribute not found in data model");
						}
						/*
						//-- 2. Проверка через ZAP-generated код
						if(!check_attribute_existence(cluster_id, attribute_id)) {
							ESP_LOGE("APP", "Attribute not found in ZAP-generated code");
						}
						*/
						/*
						//-- 3. Получение списка всех атрибутов endpoint
						print_all_attributes(endpoint_id);
						*/
						/*
						//-- 4. Проверка через Matter API
						if(!matter_attribute_exists(endpoint_id, cluster_id, attribute_id)) {
							ESP_LOGE("APP", "Attribute not found in Matter");
						}
						*/
						/*
						//-- 5. Комплексная проверка с валидацией типа
						if(!validate_attribute(endpoint_id, cluster_id, attribute_id, ESP_MATTER_VAL_TYPE_UINT32)) {
							ESP_LOGE("APP", "Attribute validation failed");
						}
						*/

            //ESP_LOGI(TAG_EMPTY, "# %s    Attribute ID: 0x%08" PRIX32, indent.c_str(), attribute_id);
            std::string attribute_name = get_attribute_name(cluster_id, attribute_id);
            //ESP_LOGI(TAG_EMPTY, "# %s    Attribute Name: %s", indent.c_str(), attribute_name.c_str());
            uint16_t flags = attribute::get_flags(attribute);
            //ESP_LOGI(TAG_EMPTY, "# %s    Attribute Flags: %d", indent.c_str(), flags);
            
            esp_matter_attr_val_t val = esp_matter_nullable_uint32(0);
						if(attribute::get_val(attribute, &val) == ESP_OK) {
              ESP_LOGI(TAG_EMPTY, "# %s    Attribute 0x%08" PRIX32 " (%s) Type: %s, Value: %s, Flags: %s", 
                      indent.c_str(), 
                      attribute_id, 
                      get_attribute_name(cluster_id, attribute_id).c_str(),
                      get_attribute_type_name(val.type).c_str(),
                      get_attribute_value(val).c_str(),
                      get_attribute_flags_string(flags).c_str());
						} else {
              //ESP_LOGE(TAG_EMPTY, "# %s    Failed to get value for attribute 0x%08" PRIX32, indent.c_str(), attribute_id);
						}
            attribute = attribute::get_next(attribute);
          }
        #endif
        
        #if SHOW_DEVICE_LOG_COMMANDS
          command_t *command = command::get_first(cluster);
          while(command) {
            uint32_t command_id = command::get_id(command);
            uint16_t flags = command::get_flags(command);
            ESP_LOGI(TAG_EMPTY, "# %s    Command 0x%08" PRIX32 ", Flags: %d", indent.c_str(), command_id, flags);
            command = command::get_next(command);
          }
        #endif

        cluster = cluster::get_next(cluster);
      }
    #endif
  }
}


//-- ПРОВЕРКА СУЩЕСТВОВАНИЯ АТРИБУТА

//-- 1. Использование API esp_matter::attribute::get()
bool is_attribute_present(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id)
{
  //esp_matter_attr_val_t val;
  attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);
  if(!attribute) {
  	ESP_LOGE(TAG_MIKE_APP, "Failed to get attribute");
  	return false;
	}
  return true;
}

//-- 2. Проверка через ZAP-generated код
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
bool check_attribute_existence(uint32_t cluster_id, uint32_t attribute_id)
{
    const EmberAfAttributeMetadata *metadata = emberAfLocateAttributeMetadata(
        0,  // Проверка на уровне кластера
        cluster_id,
        attribute_id);
    
    return metadata != nullptr;
}

//-- 3. Получение списка всех атрибутов endpoint
#include <app/util/endpoint-config-api.h>
void print_all_attributes(uint16_t endpoint_id)
{
  const EmberAfEndpointType *endpoint_type = emberAfFindEndpointType(endpoint_id);
  if(!endpoint_type) {
    ESP_LOGE("APP", "Endpoint %d not found", endpoint_id);
    return;
  }

  int level = 0;
  std::string indent(level * 2, ' ');
  uint32_t attribute_id, cluster_id;

  ESP_LOGE(TAG_EMPTY, "# %sEndpoint 0x%04" PRIX16, indent.c_str(), endpoint_id);

  #if SHOW_DEVICE_LOG_CLUSTERS
    for(size_t cluster_idx = 0; cluster_idx < endpoint_type->clusterCount; ++cluster_idx) {
      const EmberAfCluster *cluster = &endpoint_type->cluster[cluster_idx];
      
      cluster_id = cluster->clusterId;

      std::string cluster_name = get_cluster_name(cluster->clusterId);
      ESP_LOGW(TAG_EMPTY, "# %s  Cluster 0x%08" PRIX32 " (%s)", 
               indent.c_str(),
               cluster_id,
               cluster_name.c_str());
      
      #if SHOW_DEVICE_LOG_ATTRIBUTES
        for(size_t attr_idx = 0; attr_idx < cluster->attributeCount; ++attr_idx) {
          const EmberAfAttributeMetadata *attr = &cluster->attributes[attr_idx];

          attribute_id = attr->attributeId;
          attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);
          if(attribute) {
            std::string attribute_name = get_attribute_name(cluster_id, attribute_id);
            uint16_t flags = attribute::get_flags(attribute);
              
            esp_matter_attr_val_t val = esp_matter_nullable_uint32(0);
						//-- if attribute size > 0 => get attribute value
						std::string attribute_value = "";
						std::string attribute_type = get_attribute_type_name(val.type);
						if(attr->size > 0 && attribute::get_val(attribute, &val) == ESP_OK) {
							//-- new value of attribute!
							attribute_value = get_attribute_value(val);
							
						}
					  
            ESP_LOGI(TAG_EMPTY, "# %s    Attribute 0x%08" PRIX32 " (%s) Type: %s, Size: %d, Value: %s, Mask: 0x%02x, Flags: %s", 
                     indent.c_str(), 
                     attribute_id, 
                     get_attribute_name(cluster_id, attribute_id).c_str(),
                     attribute_type.c_str(),
                     attr->size,
                     attribute_value.c_str(),
                     attr->mask,
                     get_attribute_flags_string(flags).c_str());
          }
        }
      #endif
    }
  #endif
}

/*
//-- 4. Проверка через Matter API
#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
bool matter_attribute_exists(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id)
{
    chip::app::ConcreteAttributePath path(endpoint_id, cluster_id, attribute_id);
    chip::app::AttributeAccessInterface *access = chip::app::AttributeAccessInterface::GetAttributeAccessInterface(path);
    return access != nullptr;
}
*/

//-- 5. Комплексная проверка с валидацией типа
bool validate_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_val_type_t expected_type)
{
    // 1. Проверка существования
    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);
  	if(!attribute) {
  		return false;
		}

    // 2. Проверка типа
    esp_matter_attr_val_t val = esp_matter_nullable_uint32(0);
    ESP_LOGI("VALIDATE", "validate_attribute()");
    if(attribute::get_val(attribute, &val) == ESP_OK) {
    	if(val.type != expected_type) {
        ESP_LOGE("VALIDATE", "Type mismatch: expected %d, got %d", expected_type, val.type);
        return false;
    	}
    }

    // 3. Дополнительные проверки для конкретных типов
    if (expected_type == ESP_MATTER_VAL_TYPE_UINT32 && val.val.u32 > UINT32_MAX) {
        return false;
    }

    return true;
}

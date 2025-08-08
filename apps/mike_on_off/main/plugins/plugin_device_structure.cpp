
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

std::string get_attribute_name(uint32_t attribute_id)
{
  auto it = ATTRIBUTE_NAMES.find(attribute_id);
  if (it != ATTRIBUTE_NAMES.end()) {
      return it->second;
  }
  return "Unknown";
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
    //{ATTRIBUTE_FLAG_READABLE,   'R'},
    {ATTRIBUTE_FLAG_WRITABLE,   'W'},
    {ATTRIBUTE_FLAG_NULLABLE,   'N'}/*,
    {ATTRIBUTE_FLAG_REPORTABLE, 'P'}
    */
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
    if(endpoint_id != CUSTOM_ENDPOINT_ID) {
    	continue;
    }

    std::string indent(level * 2, ' ');
    ESP_LOGE(TAG_EMPTY, "# %sEndpoint 0x%04" PRIX16, indent.c_str(), endpoint_id);

    cluster_t *cluster = cluster::get_first(endpoint);
    while(cluster) {
      uint32_t cluster_id = cluster::get_id(cluster);
      std::string cluster_name = get_cluster_name(cluster_id);
      
      ESP_LOGW(TAG_EMPTY, "# %s  Cluster 0x%08" PRIX32 " (%s)", 
              indent.c_str(), cluster_id, cluster_name.c_str());

      attribute_t *attribute = attribute::get_first(cluster);
      while(attribute) {
        uint32_t attribute_id = attribute::get_id(attribute);
        std::string attribute_name = get_attribute_name(attribute_id);
        
        esp_matter_attr_val_t val;
				if(esp_matter::attribute::get_val(attribute, &val) == ESP_OK) {
          ESP_LOGI(TAG_EMPTY, "# %s    Attribute 0x%08" PRIX32 " (%s) Type: %s, Value: %s, Flags: %s", 
                  indent.c_str(), 
                  attribute_id, 
                  get_attribute_name(attribute_id).c_str(),
                  get_attribute_type_name(val.type).c_str(),
                  get_attribute_value(val).c_str(),
                  get_attribute_flags_string(esp_matter::attribute::get_flags(attribute)).c_str());
				} else {
          ESP_LOGE(TAG_EMPTY, "# %s    Failed to get value for attribute 0x%08" PRIX32, 
                  indent.c_str(), attribute_id);
				}
        
        attribute = attribute::get_next(attribute);
      }

      /*
      command_t *command = command::get_first(cluster);
      while(command) {
        uint32_t command_id = command::get_id(command);
        ESP_LOGI(TAG_EMPTY, "%s    Command 0x%08" PRIX32, indent.c_str(), command_id);
        command = command::get_next(command);
      }
      */
      cluster = cluster::get_next(cluster);
    }
  }
}

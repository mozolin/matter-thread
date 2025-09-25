<?
namespace app\helpers;

class SdkConfig
{
	private static $PATH_OTBR_COMPONENTS;
	private static $PATH_OTBR_EXAMPLE;
	private static $FILE_SDKCONFIG;
	private static $FILE_SDKCONFIG_DEFAULTS;

	private static function init()
	{
		self::$PATH_OTBR_COMPONENTS = \Yii::$app->params["PATH_OTBR_COMPONENTS"];
		self::$PATH_OTBR_EXAMPLE = \Yii::$app->params["PATH_OTBR_EXAMPLE"];
		self::$FILE_SDKCONFIG = \Yii::$app->params["FILE_SDKCONFIG"];
		self::$FILE_SDKCONFIG_DEFAULTS = self::$PATH_OTBR_EXAMPLE.'/'.self::$FILE_SDKCONFIG;
	}
	
	private static $sdkConfigParams = [
	  "ESP32-S3" => [
	  	"status" => -1,
	  	"params" => [
		  	"CONFIG_IDF_TARGET" => [
		  		"custom" => 0,
		  		"value" => "esp32s3",
		  	],
				"CONFIG_OPENTHREAD_BR_AUTO_START" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_OPENTHREAD_BR_START_WEB" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
	  "Custom Firmware Config" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_MIKE_MDNS_HOSTNAME" => [
					"custom" => 1,
					"value" => "esp-ot-br",
		  	],
				"CONFIG_MIKE_DEVICE_ID" => [
					"custom" => 1,
					"value" => "esp-ot-br",
		  	],
				"CONFIG_MIKE_FIRMWARE_VERSION" => [
					"custom" => 1,
					"value" => "1.0.0",
		  	],
				/*
				"CONFIG_APP_PROJECT_VER_FROM_CONFIG" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_APP_PROJECT_VER" => [
					"custom" => 1,
					"value" => "1.0.0",
		  	],
		  	*/
			],
	  ],
		"Serial flasher config" => [
	  	"status" => -1,
	  	"params" => [
			  "CONFIG_ESPTOOLPY_FLASHSIZE_4MB" => [
			  	"custom" => 1,
			  	"value" => "y",
		  	],
			],
	  ],
		"Partition Table" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_PARTITION_TABLE_CUSTOM" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME" => [
					"custom" => 0,
					"value" => "partitions.csv",
		  	],
				"CONFIG_PARTITION_TABLE_FILENAME" => [
					"custom" => 0,
					"value" => "partitions.csv",
		  	],
				"CONFIG_PARTITION_TABLE_OFFSET" => [
					"custom" => 0,
					"value" => "0x8000",
		  	],
				"CONFIG_PARTITION_TABLE_MD5" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
		"Compiler options" => [
	  	"status" => -1,
	  	"params" => [
			  "CONFIG_COMPILER_OPTIMIZATION_SIZE" => [
			  	"custom" => 0,
			  	"value" => "y",
		  	],
			],
	  ],
		"Newlib" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_NEWLIB_NANO_FORMAT" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
		"mbedTLS" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_MBEDTLS_CMAC_C" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_MBEDTLS_SSL_PROTO_DTLS" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_MBEDTLS_KEY_EXCHANGE_ECJPAKE" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_MBEDTLS_ECJPAKE_C" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
		"OpenThread" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_OPENTHREAD_ENABLED" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_OPENTHREAD_BORDER_ROUTER" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_OPENTHREAD_CLI_OTA" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_OPENTHREAD_RCP_COMMAND" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_OPENTHREAD_RADIO_SPINEL_UART" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
		"OpenThread Border Router Example" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_AUTO_UPDATE_RCP" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
		"lwIP" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_LWIP_IPV6_FORWARD" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_IPV6_NUM_ADDRESSES" => [
					"custom" => 0,
					"value" => "12",
		  	],
				"CONFIG_LWIP_MULTICAST_PING" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_NETIF_STATUS_CALLBACK" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_HOOK_IP6_ROUTE_DEFAULT" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_HOOK_ND6_GET_GW_DEFAULT" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_HOOK_IP6_INPUT_CUSTOM" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_IPV6_AUTOCONFIG" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_HOOK_IP6_SELECT_SRC_ADDR_CUSTOM" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_LWIP_FORCE_ROUTER_FORWARDING" => [
					"custom" => 0,
					"value" => "y",
		  	],
			],
	  ],
		"mDNS" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_MDNS_MULTIPLE_INSTANCE" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_MDNS_MAX_SERVICES" => [
					"custom" => 0,
					"value" => "200",
		  	],
			],
	  ],
		"ESP System Settings" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE" => [
					"custom" => 0,
					"value" => "3584",
		  	],
			],
	  ],
		"ESP Border Router Board Kit" => [
	  	"status" => -1,
	  	"params" => [
			"CONFIG_ESP_BR_BOARD_DEV_KIT" => [
				"custom" => 0,
				"value" => "y",
		  	],
			"CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG" => [
				"custom" => 0,
				"value" => "y",
		  	],
			],
	  ],
		"Ethernet" => [
	  	"status" => 0,
	  	"params" => [
				"CONFIG_EXAMPLE_CONNECT_ETHERNET" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_EXAMPLE_ETHERNET_EMAC_TASK_STACK_SIZE" => [
					"custom" => 0,
					"value" => "2048",
		  	],
				"CONFIG_EXAMPLE_USE_SPI_ETHERNET" => [	
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_EXAMPLE_USE_W5500" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_HOST" => [
					"custom" => 0,
					"value" => "2",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO" => [
					"custom" => 0,
					"value" => "21",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO" => [
					"custom" => 0,
					"value" => "45",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO" => [
					"custom" => 0,
					"value" => "38",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_CS_GPIO" => [
					"custom" => 0,
					"value" => "41",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ" => [
					"custom" => 0,
					"value" => "36",
		  	],
				"CONFIG_EXAMPLE_ETH_SPI_INT_GPIO" => [
					"custom" => 0,
					"value" => "39",
		  	],
				"CONFIG_EXAMPLE_ETH_PHY_RST_GPIO" => [
					"custom" => 0,
					"value" => "40",
		  	],
				"CONFIG_EXAMPLE_ETH_PHY_ADDR" => [
					"custom" => 0,
					"value" => "1",
		  	],
			],
	  ],
		"Wi-Fi" => [
	  	"status" => 1,
	  	"params" => [
				"CONFIG_EXAMPLE_CONNECT_WIFI" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_EXAMPLE_PROVIDE_WIFI_CONSOLE_CMD" => [
					"custom" => 0,
					"value" => "y",
		  	],
				"CONFIG_EXAMPLE_WIFI_SSID" => [
					"custom" => 1,
					"value" => "SOME_WIFI_24",
		  	],
				"CONFIG_EXAMPLE_WIFI_PASSWORD" => [
					"custom" => 1,
					"value" => "some-pwd",
		  	],
				"CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY" => [
					"custom" => 0,
					"value" => "1000000",
		  	],
			],
	  ],
		"http server" => [
	  	"status" => -1,
	  	"params" => [
				"CONFIG_HTTPD_MAX_REQ_HDR_LEN" => [
					"custom" => 0,
					"value" => "1024",
		  	],
			],
	  ],
	];

	
	public static function getParamsList()
	{
		return self::$sdkConfigParams;
	}

	public static function getSwitchableSectionsList()
	{
		$switchableSections = [];
		foreach(self::$sdkConfigParams as $secname => $section) {
			$status = $section['status'];
			if($status === 0 || $status === 1) {
				$switchableSections[$secname] = $section;
			}
		}
		return $switchableSections;
	}

	public static function getSwitchableSectionNamesList()
	{
		$switchableSectionNames = [];
		foreach(self::$sdkConfigParams as $secname => $section) {
			$status = $section['status'];
			if($status === 0 || $status === 1) {
				$switchableSectionNames[] = $secname;
			}
		}
		return $switchableSectionNames;
	}

	
	public static function getCustomSectionsList()
	{
		$customSections = [];
		foreach(self::$sdkConfigParams as $secname => $section) {
			$params = $section['params'];
			foreach($params as $paramName => $param) {
				if($param['custom']) {
					if(!array_key_exists($secname, $customSections)) {
						$customSections[$secname] = [
							'status' => $section['status'],
						];
					}
					//-- add parameter
					$customSections[$secname]['params'][$paramName] = $param;
				}
			}
		}
		return $customSections;
	}

	public static function checkSectionRealStatus($section)
	{
		$switchableSectionNames = self::getSwitchableSectionNamesList();
		$config = self::getSdkConfigList();
		
		foreach($switchableSectionNames as $switchableSectionName) {
			foreach($config as $cnf) {
				if($cnf['section'] === $section) {
					//-- section found => return the status of the first parameter
					//-- it means, that all other parameters in this section have the same status
					return $cnf['status'];
				}
			}
		}
	}

	public static function checkParameterRealStatus($param)
	{
		$param = strip_tags($param);
		$pregCond = "{(CONFIG_ESPTOOLPY_FLASHSIZE_)(\d+)(MB)}si";
		
		$config = self::getSdkConfigList();
		foreach($config as $item) {
			if(preg_match($pregCond, $param) && preg_match($pregCond, $item['name'], $m)) {
				return $m[2];
			} elseif($item['name'] === $param) {
				return str_replace('"', '', $item['value']);
			}
		}
		return null;
	}
	
	public static function getSdkConfigList()
	{
		self::init();
		
		$config = [];
		$section = '';
	  
		$fn = self::$FILE_SDKCONFIG_DEFAULTS;
		
		if(file_exists($fn)) {
			$contents = file($fn);
			foreach($contents as $row) {
				$row = trim($row);
				if(substr($row, 0, 1) === '#') {
					if(substr($row, 0, 8) === '#CONFIG_') {
						//-- disabled parameter
						//echo "<input type=\"checkbox\">{$row}<br/>";
						if(preg_match("{(.*?)=(.*)}si", $row, $m)) {
							$config[] = [
								'section' => $section,
								'status' => 0,
								'name'   => substr($m[1], 1),
								'value'  => $m[2],
							];
						}
					} else {
						//-- comment
						//echo "{$row}<br/>";
						if(preg_match("{begin of (.*)}si", $row, $m)) {
							//-- new section
							$section = $m[1];
						}
						/*
						$config[] = [
							'section' => $section,
							'status'  => -1,
							'name'    => '',
							'value'   => substr($row, 1),
						];
						*/
						if(preg_match("{end of (.*)}si", $row, $m)) {
							//-- end of section
							$section = '';
						}
					}
				} elseif(strlen($row) === 0) {
					//-- empty row
					//echo "{$row}<br/>";
					//$config[] = [
					//	'section' => $section,
					//	'status'  => -2,
					//	'name'    => '',
					//	'value'   => '',
					//];
				} else {
					//-- enabled parameter
					//echo "<input type=\"checkbox\" checked>{$row}<br/>";
					if(preg_match("{(.*?)=(.*)}si", $row, $m)) {
						$config[] = [
							'section' => $section,
							'status'  => 1,
							'name'    => $m[1],
							'value'   => $m[2],
						];
					}
				}
			}
		} else {
			//echo "NOT FOUND!";
		}
	  
	  return $config;
	}
	
	public static function saveSdkConfigList()
	{
		self::init();
		
		$config = self::getSdkConfigList();
		
		$data = '';
		foreach($config as $cfg) {
			$status = $cfg['status'];
			$name = $cfg['name'];
			$value = $cfg['value'];
			switch($status) {
				//-- empty row
				case -2:
					//$data .= "\r\n";
					break;
				//-- comment
				case -1:
					$data .= "#{$value}\r\n";
					if(preg_match("{end of (.*)}si", $value, $m)) {
						//-- end of section => add an empty row
						$data .= "\r\n";
					}
					break;
				//-- disabled parameter
				case 0:
					$data .= "#{$name}={$value}\r\n";
					break;
				//-- enabled parameter
				case 1:
				  $data .= "{$name}={$value}\r\n";
					break;
			}
		}
	  
	  $fn = self::$FILE_SDKCONFIG_DEFAULTS.'_test';

		if(@file_put_contents($fn, $data)) {
			return true;
		}

		return false;
	}

	public static function saveParamsList($formData)
	{
		self::init();
		
		$data = '';
		
		foreach(self::$sdkConfigParams as $secname => $section) {
			$status = $section['status'];
			$chkSec = 'CONFIG_SWITCHABLE_SECTIONS_'.$secname;
			//-- check for the Switchable Sections
			if(array_key_exists($chkSec, $formData)) {
				$status = $formData[$chkSec];
			}
			//-- add "# begin of <section>"
			$data .= "# begin of {$secname}\r\n";
			$params = $section['params'];
			foreach($params as $paramName => $param) {
				$val = trim($param['value']);

				if(preg_match("{(CONFIG_ESPTOOLPY_FLASHSIZE_)(\d+)(MB)}si", $paramName, $m) && array_key_exists('CONFIG_ESPTOOLPY_FLASHSIZE', $formData)) {
					$val = 'y';
					$paramName = $m[1].$formData['CONFIG_ESPTOOLPY_FLASHSIZE'].$m[3];
				} else {
					//-- replace with a new value
					if(array_key_exists($paramName, $formData)) {
						$val = $formData[$paramName];
					}
				  //-- correct syntax of value
					if($val !== 'y' && !is_numeric($val)) {
						$val = '"'.$val.'"';
					}
					//-- disabled section
					if($status == 0) {
						//-- put a remark in the beginning of the row
						$data .= "#";
					}
				}
				//-- add parameter: name + value
				$data .= "{$paramName}={$val}\r\n";
				//print_r($param);
			}
			//-- add "# end of <section>"
			$data .= "# end of {$secname}\r\n";
			//-- add an empty row
			$data .= "\r\n";
		}

		$fn = self::$FILE_SDKCONFIG_DEFAULTS;
		@unlink($fn);

		$fsize = @file_put_contents($fn, $data);
		if($fsize !== false) {
			if(file_exists($fn)) {
				return $fsize;
			}
		}

		return 0;
	}

}

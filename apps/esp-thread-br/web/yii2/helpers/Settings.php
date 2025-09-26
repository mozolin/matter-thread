<?
namespace app\helpers;

class Settings
{
	private static $_PATH_OTBR_COMPONENTS;
	private static $_PATH_OTBR_EXAMPLE;
	public static $_FILE_SDKCONFIG;
	public static $_FILE_SDKCONFIG_DEFAULTS;

	public static $SWITCHABLE_SECTIONS = 'CONFIG_SWITCHABLE_SECTIONS_';

	public static $_FILE_PARTITIONS;
	public static $_FILE_PARTITIONS_CSV;

	public static function _init()
	{
		self::$_PATH_OTBR_COMPONENTS = \Yii::$app->params["PATH_OTBR_COMPONENTS"];
		self::$_PATH_OTBR_EXAMPLE = \Yii::$app->params["PATH_OTBR_EXAMPLE"];
		self::$_FILE_SDKCONFIG = \Yii::$app->params["FILE_SDKCONFIG"];
		self::$_FILE_SDKCONFIG_DEFAULTS = self::$_PATH_OTBR_EXAMPLE.'/'.self::$_FILE_SDKCONFIG;

		self::$_FILE_PARTITIONS = \Yii::$app->params["FILE_PARTITIONS"];
		self::$_FILE_PARTITIONS_CSV = self::$_PATH_OTBR_EXAMPLE.'/'.self::$_FILE_PARTITIONS;
	}

}

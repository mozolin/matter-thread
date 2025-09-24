<?
namespace app\helpers;

class EspTool
{
	
	public static function getFlashId($comPort)
	{
		$command = "esptool -p {$comPort} flash-id";
		$output = shell_exec($command);
		
		return $output;
	}

}

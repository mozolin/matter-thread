<?
namespace app\helpers;

use app\helpers\Settings;

class PartitionsCsv
{
	private static $_bootLoaderBin = 64 * 1024;
	private static $_partitionTableBin = 4 * 1024;
	private static $_partitionTableOffset = 0x8000;

	public static function getPartitionsCsvInfo()
	{
		$rows = file(Settings::$_FILE_PARTITIONS_CSV);
		$totalSize = 0;

		$data = [
			'table'       => [],
			'total-size'  => 0,
		];

		foreach($rows as $row) {
			$row = trim($row);
			
			$isOTA = preg_match("{ota_\d+}si", $row );
			$disabledOTA = (substr($row, 0, 5) === '#ota_');
			
			if(substr($row, 0, 1) !== '#' || $isOTA) {
				list($name, $type, $subtype, $offset, $size, $flags) = explode(',', $row);
				$sizeBytes = $size;
				if(preg_match("{0x\d+}si", $sizeBytes, $m)) {
					//-- hex
					$sizeBytes = hexdec($sizeBytes);
				} elseif(preg_match("{(|0x)(\d+)(k|M)}si", $size, $m)) {
					//-- kilobytes
					if(strtolower($m[3]) === 'k') {
						$sizeBytes = $m[2] * 1024;
					}
					//-- megabytes
					if(strtolower($m[3]) === 'm') {
						$sizeBytes = $m[2] * 1024 * 1024;
					}
				}
				if(!$disabledOTA) {
					$totalSize += $sizeBytes;
				}

				$data['table'][] = [
					'name'      => $name,
				  'type'      => $type,
				  'subtype'   => $subtype,
				  'size'      => $size,
				  'sizeBytes' => $sizeBytes,
				];
			}
		}
		
		$extraSize = self::$_bootLoaderBin + self::$_partitionTableBin + self::$_partitionTableOffset;
		$totalSize += $extraSize;
		
		$data['total-size'] = $totalSize;
		return $data;
	}

	public static function savePartitionsCsv($formData)
	{
		Settings::_init();	

		$fn = Settings::$_FILE_PARTITIONS_CSV;
		$contents = file_get_contents($fn);

		foreach($formData as $name => $val) {
			$pos = strpos($contents, "#{$name}");
			if($pos !== false) {
				//-- was: disabled
				if($val == 1) {
					//-- now: enabled
					$contents = str_replace("#{$name}", $name, $contents);
				} else {
					//-- do not change
				}
			} else {
				//-- was: enabled
				if($val == 0) {
					//-- now: disabled
					$contents = str_replace($name, "#{$name}", $contents);
				} else {
					//-- do not change
				}
			}
		}

		$fsize = file_put_contents($fn, $contents);
		if($fsize !== false) {
			if(file_exists($fn)) {
				return $fsize;
			}
		}

		return 0;
	}

}

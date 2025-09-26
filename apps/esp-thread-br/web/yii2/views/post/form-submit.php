<?
use app\helpers\SdkConfig;

$result = [];

$switchableSectionsFound = false;
$chkSec = SdkConfig::$SWITCHABLE_SECTIONS;
foreach($data as $name => $value) {
	if(substr($name, 0, strlen($chkSec)) === $chkSec) {
		if($value == 1) {
			$switchableSectionsFound = true;
			break;
		}
	}
}

if(!$switchableSectionsFound) {
	$result['status'] = 'error';
	$result['message'] = 'You must select at least one of the switchable sections!';
} else {
	$fsize = SdkConfig::saveParamsList($data);
	if($fsize > 0) {
		$result['status'] = 'success';
		$result['fsize'] = $fsize;
	} else {
		$result['status'] = 'error';
		$result['message'] = 'Error occured!';
	}
}

echo json_encode($result);

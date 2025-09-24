<?
use app\helpers\EspTool;

//-- The "comPort" parameter is received from the controller
$flashId = EspTool::getFlashId("COM".$comPort);


$esp32ChipFound = false;
$esp32ChipRAM = "";

$ESP32_CHIP = \Yii::$app->params["ESP32_CHIP"];

if(preg_match("{Detecting chip type...(.*?)\n}si", $flashId, $m)) {
	$esp32Chip = strtoupper(trim($m[1]));
	if(!empty($esp32Chip)) {
		if($esp32Chip === $ESP32_CHIP) {
			$esp32ChipFound = true;
		}
	}
}
if($esp32ChipFound) {
	if(preg_match("{Detected flash size:.*?(\d+)MB}si", $flashId, $m)) {
		$esp32ChipRAM = strtoupper(trim($m[1]));
	}
}

$data = [
	'port' => $comPort,
];

if($esp32ChipFound && !empty($esp32ChipRAM)) {
	$data = [
		'port' => $comPort,
		'chip' => $ESP32_CHIP,
		'ram'  => $esp32ChipRAM,
	];
}

echo json_encode($data);

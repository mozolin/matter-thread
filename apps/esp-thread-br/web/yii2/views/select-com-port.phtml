<?
$command = "wmic path Win32_PnPEntity where \"ConfigManagerErrorCode = 0 and Caption like '%(COM%)'\" get Caption /value";
$output = shell_exec($command);

$utf8Output = mb_convert_encoding($output, "UTF-8", "CP866");
preg_match_all('/Caption=(.*?)\(COM(\d+)\)/', $utf8Output, $matches);

$comPorts = [];

if(!empty($matches[2])) {
	$numItems = count($matches[2]);
	for($i=0;$i<$numItems;$i++) {
		$comPortNum = $matches[2][$i];
		$comPortName = $matches[1][$i];
		$comPorts[$comPortNum] = $comPortName;
	}

	ksort($comPorts);

	?>
	<div id="div-get-esp32-chip-info" name="get-esp32-chip-info">
		<b>Get ESP32 Chip Info</b><br/>
		<div id="div-over-select">
			<select id="select-com-ports-list" class="frm">
				<?
				foreach($comPorts as $comPortNum => $comPortName) {
					$optStr = "COM{$comPortNum}: {$comPortName}";
					?>
					<option value="<?=$comPortNum?>"><?=$optStr?></option>
					<?
				}
				?>
			</select><button id="button-esptool-flash-id" class="btn btn-primary">Get FlashID</button>
			<div id="div-status"></div>
		</div>
	</div>
	<?
}

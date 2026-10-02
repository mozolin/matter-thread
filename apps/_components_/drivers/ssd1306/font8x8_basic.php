<?
$data = [0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x00]; //-- 4
foreach($data as $item) {
	//echo hex2bin(substr($item, 2))."\n";
	$s = str_pad(decbin($item), 8, "0", STR_PAD_LEFT);
	$s = str_replace(["0", "1"], [".", "#"], $s);
	echo $s."\n";
}
